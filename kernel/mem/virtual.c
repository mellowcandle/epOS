/*
	This is free and unencumbered software released into the public domain.

	Anyone is free to copy, modify, publish, use, compile, sell, or
	distribute this software, either in source code form or as a compiled
	binary, for any purpose, commercial or non-commercial, and by any

	In jurisdictions that recognize copyright laws, the author or authors
	of this software dedicate any and all copyright interest in the
	software to the public domain. We make this dedication for the benefit
	of the public at large and to the detriment of our heirs and
	successors. We intend this dedication to be an overt act of
	relinquishment in perpetuity of all present and future rights to this
	software under copyright law.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
	EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
	MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
	IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
	OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
	ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
	OTHER DEALINGS IN THE SOFTWARE.

	For more information, please refer to <http://unlicense.org>
*/


/*
 * Memory map:
 *
 * 0x00000000-0xC0000000 User space usage
 * 0xC0000000-0xD0000000 Kernel (256 MB to grow to....)
 * 0xD0000000-0xE0000000 Kernel heap
 * 0xE0000000........... Unused (for future use)
 *
 */
//#define DEBUG

#include <mem/memory.h>
#include <cpu.h>
#include <printk.h>
#include <types.h>
#include <isr.h>
#include <kmalloc.h>
#include <lib/string.h>
#include <kernel.h>
#include <bits.h>

/* Page directory table                       Page table                                   Physical memory    */

/* +-----------------+                    +------------------^                           +------------------+ */
/* |      PDE        +-------------------->       PTE        +------------------+        |                  | */
/* +-----------------+                    +------------------+                  |        |                  | */
/* |                 |                    |                  +-------------+    +-------^+                  | */
/* +-----------------+                    +------------------+             |             |                  | */
/* |                 |                    |                  |             |             +------------------+ */
/* +-----------------+                    +------------------+             |             |                  | */
/* |                 |                    |                  |             |             |                  | */
/* +-----------------+                    +------------------+             +------------->                  | */
/* |                 |                    |                  |                           |                  | */
/* +-----------------+                    +------------------+                           |                  | */
/* |                 |                    |                  |                           +------------------+ */
/* |                 |                    |                  |                           |                  | */
/* |                 |                    |                  |                           |                  | */
/* |                 |                    |                  |                           |                  | */
/* |                 |                    |                  |                           |                  | */
/* |                 |                    |                  |                                                */

/*	PDE = Page directory entry
	PTE = Page table entry
*/

/* We can directly access the PDE entries as they we're already mapped to virtual space with the kernel */
/* When accessing using mirror method, this gives us a virtual pointer to the PTE's. we can then set, clear and adjust them */

/* PDT & PDE definitions */



uint32_t *current_pdt = (uint32_t *) PDT_MIRROR_BASE;

addr_t virt_to_phys(void *addr)
{
	uint32_t *pte;
	uint32_t pde_idx = FRAME_TO_PDE_INDEX((addr_t) addr);
	uint32_t pte_idx = FRAME_TO_PTE_INDEX((addr_t) addr);

	assert(IS_PAGE_ALIGNED(addr));

	if (current_pdt[pde_idx] & PDT_PRESENT) {
		if (current_pdt[pde_idx] & PDT_HUGE_PAGE)
			return (current_pdt[pde_idx] & PAGE_MASK) + ((addr_t)addr & HUGE_PAGE_MASK);
		else {
			pte = (uint32_t *)(PDE_MIRROR_BASE + (pde_idx * 0x1000));
			pte += pte_idx; // int pointer aritmethics here.
			return (*pte & PAGE_MASK);
		}
	}

	return 0;
}



// These are defined in the linker script, with these we calculate the size of the kernel on runtime.
extern uint32_t kernel_end, kernel_start;
void dump_pdt();

void page_fault_handler(registers_t *regs);
void mem_heap_init();
void mem_phys_init(addr_t phy_start, uint32_t total_pages);
static void _kernel_allocate_pdt();

void mem_switch_page_directory(addr_t new_dir)
{
	FUNC_ENTER();

//	pr_warn("setting page directory to: 0x%X\r\n", new_dir);
	__asm volatile("mov %0, %%cr3":: "r"(new_dir));

	FUNC_LEAVE();

}


void dump_pdt()
{
	char *ptr;
	addr_t virtual_pos;

	for (int i = 0; i < PAGE_DIRECTORY_SIZE; i++) {
		if (current_pdt[i] & PDT_PRESENT) { // exists
			printk("PDE: %u value: 0x%x\r\n", i, current_pdt[i]);

			if (current_pdt[i] & PDT_HUGE_PAGE)
				printk("Maps 4MB page from 0x%x to 0x%x\r\n", PDE_INDEX_TO_ADDR(i), PDE_INDEX_TO_ADDR(i + 1));
			else {
				ptr = (char *) PDE_MIRROR_BASE + (i * 0x1000);

				for (int j = 0; j < PAGE_TABLE_SIZE; j++) {
					if (* ((uint32_t *)ptr) & PTE_PRESENT) {
						virtual_pos = PDE_INDEX_TO_ADDR(i) + (j * PAGE_SIZE);
						printk("\tPTE: %u value: 0x%x maps 4K page from 0x%x to 0x%x\r\n", j, * (uint32_t *) ptr,
						       virtual_pos, virtual_pos + PAGE_SIZE);
					}

					ptr += 4;

				}
			}

		}
	}
}

void dump_pdt_indirect(uint32_t *target_pdt)
{
	addr_t virtual_pos;
	uint32_t *temp;
	printk("Dumping PDT: mapped to: 0x%x\r\n", (uint32_t) target_pdt);

	for (int i = 0; i < PAGE_DIRECTORY_SIZE; i++) {
		if (target_pdt[i] & PDT_PRESENT) { // exists
			printk("PDE: %u value: 0x%x\r\n", i, current_pdt[i]);

			if (target_pdt[i] & PDT_HUGE_PAGE)
				printk("Maps 4MB page from 0x%x to 0x%x\r\n", PDE_INDEX_TO_ADDR(i), PDE_INDEX_TO_ADDR(i + 1));
			else {
				temp = mem_page_map_kernel_single(target_pdt[i] & PAGE_MASK, READ_ONLY_KERNEL);

				for (int j = 0; j < PAGE_TABLE_SIZE; j++) {
					if (*temp & PTE_PRESENT) {
						virtual_pos = PDE_INDEX_TO_ADDR(i) + (j * PAGE_SIZE);
						printk("\tPTE: %u value: 0x%x maps 4K page from 0x%x to 0x%x\r\n", j, *temp,
						       virtual_pos, virtual_pos + PAGE_SIZE);
					}

					temp++;

				}
			}

		}
	}
}

void *mem_page_map_kernel(addr_t physical, int count, int flags)
{
	void *addr = mem_find_kernel_place(count);

	if (addr) {
		for (int i = 0 ; i < count; i++)
			mem_page_map(physical + (i * PAGE_SIZE), addr + (i * PAGE_SIZE), flags);

		return (void *) addr;
	}

	pr_fatal("No more virtual kernel space to satisfy request\r\n");
	panic();

	return NULL;
}

void *mem_find_kernel_place(int count)
{
	char *ptr;
	void *addr;
	int n = 0;
	bool begin = false;
	uint32_t start_i = 0;
	uint32_t start_j = 0;

	/*  Algorithm:
	 *  This is pretty easy. we search the PDE & PDT for the first matching "request" pages.
	 *  When we find the first empty page, we mark the pos in start_i and start_j and set begin to true.
	 *  If we manage to count N consecutive pages we return the address sucessfuly */

	for (uint32_t i = FRAME_TO_PDE_INDEX(KERNEL_VIRTUAL_BASE); i < PAGE_DIRECTORY_SIZE; i++) {
		if (current_pdt[i] & PDT_PRESENT) { // exists
			if (current_pdt[i] & PDT_HUGE_PAGE) {
				if (begin) {
					begin = false;
					n = 0;
				}

				continue;
			} else {
				ptr = (char *) PDE_MIRROR_BASE + (i * 0x1000);

				for (uint32_t j = 0; j < PAGE_TABLE_SIZE; j++, ptr += 4) {
					if (* ((uint32_t *)ptr) & PTE_PRESENT) {
						if (begin) {
							begin = false;
							n = 0;
						}

						continue;
					} else {
						if (!begin) {
							start_i = i;
							start_j = j;
							begin = true;
						}

						if (++n >= count)
							break;
					}
				}

			}
		} else {
			// PDE is not maped in, this means we have 1024 empty entries in here.
			if (!begin) {
				start_i = i;
				begin = true;
				start_j = 0;
			}

			n += 1024;
		}

		if (n >= count) {
			addr = (void *) PDE_INDEX_TO_ADDR(start_i) + (start_j * PAGE_SIZE);
			pr_debug("Found place for %u pages starting from 0x%x!\r\n", count, (int) addr);
			return addr;
		}

	}

	return 0;

}

void mem_init(multiboot_info_t *mbi)
{
	uint32_t total_memory;
	uint32_t physical_start;
	uint32_t kernel_size;
	uint32_t required_kernel_pages;

	FUNC_ENTER();
	register_interrupt_handler(14, page_fault_handler);

	pr_info("Memory init:\r\ndetecting physical memory.\r\n");

	multiboot_memory_map_t *mmap = (multiboot_memory_map_t *)(mbi->mmap_addr + KERNEL_VIRTUAL_BASE);

	while ((uint32_t) mmap < (mbi->mmap_addr + KERNEL_VIRTUAL_BASE +  mbi->mmap_length)) {
		mmap = (multiboot_memory_map_t *)((unsigned int)mmap + mmap->size + sizeof(unsigned int));

		if (mmap->size == 0) {
			// That's the end of the list
			break;
		}

		pr_debug("Memory region address: 0x%llx length: 0x%llx type: %u\r\n",
		         mmap->addr, mmap->len, mmap->type);

		// We only support one zone, we'll take the biggest.
		//
		if (mmap->type == 1) {
			if (mmap->len > total_memory)
				total_memory = mmap->len;
		}

	}

	if (total_memory == 0) {
		pr_fatal("No physical memory!\r\n");
		panic();
	}

	kernel_size = ((uint32_t) &kernel_end - (uint32_t) &kernel_start);
	required_kernel_pages = (kernel_size / PAGE_SIZE) + 1;

	pr_debug("Kernel start: 0x%x, kernel end: 0x%x\r\n", (uint32_t) &kernel_start,
	         (uint32_t) &kernel_end);
	pr_debug("Kernel occupies 0x%x bytes, consuming %u pages\r\n", kernel_size, required_kernel_pages);

	physical_start = ((uint32_t) &kernel_start) - KERNEL_VIRTUAL_BASE;
	physical_start += required_kernel_pages * PAGE_SIZE;
	physical_start = PAGE_ALIGN_UP(physical_start);
	total_memory -= required_kernel_pages * PAGE_SIZE;

	mem_phys_init(physical_start, total_memory);
	_kernel_allocate_pdt();
	mem_heap_init(get_kernel_heap(), KERNEL_HEAP_VIR_START, KERNEL_HEAP_SIZE);

	FUNC_LEAVE();
}

static void _kernel_allocate_pdt()
{

}

void page_fault_handler(registers_t *regs)
{

	addr_t cr2, err_code = regs->err_code;

	bool page_present; // else missing
	bool page_read;  // else write
	bool page_user;  // else kernel

	__asm__ volatile("mov %%cr2, %0" : "=r"(cr2));

	if (err_code & 1)
		page_present = true;
	else
		page_present = false;

	if (err_code & (1 << 1))
		page_read = false;
	else
		page_read = true;

	if (err_code & (1 << 2))
		page_user = true;
	else
		page_user = false;


	printk("Page fault: linear address: 0x%x\r\n", cr2);
	printk("Details: Page %s, ", page_present ? "present" : "not present");
	printk("Ocurred on %s, ", page_read ? "read" : "write");
	printk("Page ownership %s\r\n", page_user ? "user" : "kernel");


	irq_reg_dump(regs);
	dump_pdt();
	panic();
}

int mem_page_map(addr_t physical, void *virtual, int flags)
{

	addr_t page;
	char *access_ptr;
	uint32_t *pte;

	pr_debug("+mem_page_map physical: 0x%x virtual 0x%x flags %X\r\n", physical, virtual, flags);


	if ((uint32_t) virtual < 0xC0000000)
		pr_warn("+mem_page_map physical: 0x%x virtual 0x%x flags %X\r\n", physical, virtual, flags);

	assert(IS_PAGE_ALIGNED(physical));

	access_ptr = (char *)(PDE_MIRROR_BASE + (FRAME_TO_PDE_INDEX((addr_t) virtual) * 0x1000));


	// Check if the PDT exists
	if (!(current_pdt[FRAME_TO_PDE_INDEX((addr_t)virtual)] & PDT_PRESENT)) {
		pr_debug("mem_map_page: PDT missing, creating and mapping\r\n");
		page = mem_get_page();

		mem_assert(page != 0);

		// Put it in PDT
		current_pdt[FRAME_TO_PDE_INDEX((addr_t)virtual)] = page | PDT_PRESENT | flags;
		// Invalidate cache
		mem_tlb_flush(access_ptr);
		// Clear the PDE table
		memset(access_ptr, 0, PAGE_SIZE);
	}

	// Insert the PTE.
	pte = (uint32_t *)(access_ptr + (FRAME_TO_PTE_INDEX((addr_t)virtual) * sizeof(uint32_t)));

	if (*pte & PTE_PRESENT) {
		if ((*pte & PTE_ADDR_MASK) != physical) {
			pr_fatal("+mem_page_map physical: 0x%x virtual 0x%x flags %X\r\n", physical, (addr_t)virtual,
			         flags);

			while (1);
		} else
			pr_warn("Identical mapping detected\r\n");
	}

	*pte = physical | flags;

	mem_tlb_flush((void *) virtual);

	FUNC_LEAVE();
	return 0;
}

int mem_page_map_pdt(uint32_t *target_pdt, addr_t physical, void *virtual, int flags)
{

	addr_t page;
	char *access_ptr;
	uint32_t *pte;

	pr_info("+mem_page_map physical: 0x%x virtual 0x%x flags %X\r\n", physical, virtual, flags);

	assert(IS_PAGE_ALIGNED(physical));

	// Check if the PDT exists
	if (!(target_pdt[FRAME_TO_PDE_INDEX((addr_t)virtual)] & PDT_PRESENT)) {
		page = mem_get_page();

		mem_assert(page != 0);

		// Put it in PDT
		target_pdt[FRAME_TO_PDE_INDEX((addr_t)virtual)] = page | PDT_PRESENT | flags;

		// Clear the PDE table
		// Temporary map the page
		access_ptr = mem_page_map_kernel(page, 1, READ_WRITE_KERNEL | PTE_TEMPORARY);
		memset(access_ptr, 0, PAGE_SIZE);
	} else {
		target_pdt[FRAME_TO_PDE_INDEX((addr_t)virtual)] |= flags;
		access_ptr = mem_page_map_kernel(target_pdt[FRAME_TO_PDE_INDEX((addr_t)virtual)] & PAGE_MASK, 1,
		                                 READ_WRITE_KERNEL | PTE_TEMPORARY);
	}

	// Insert the PTE.
	pte = (uint32_t *)(access_ptr + (FRAME_TO_PTE_INDEX((addr_t)virtual) * sizeof(uint32_t)));

	if (*pte & PTE_PRESENT) {
		if ((*pte & PTE_ADDR_MASK) != physical) {
			pr_fatal("+mem_page_map virtual: 0x%x existing physical 0x%x physical: 0x%x flags %X\r\n",
			         (addr_t)virtual, *pte & PTE_ADDR_MASK, physical, flags);

			printk("Dumping PDT\r\n------------------\r\n");
			dump_pdt_indirect(target_pdt);

			while (1);
		} else
			pr_warn("Identical mapping detected\r\n");
	}

	*pte = physical | flags;

	mem_page_unmap(access_ptr);
	FUNC_LEAVE();
	return 0;
}

void mem_page_unmap(void *virtual)
{
	char *access_ptr;
	uint32_t *pte;

	//TODO: unmap page directory if necessary
	FUNC_ENTER();

	access_ptr = (char *)(PDE_MIRROR_BASE + (FRAME_TO_PDE_INDEX((addr_t)virtual) * 0x1000));

	pte = (uint32_t *)(access_ptr + (FRAME_TO_PTE_INDEX((addr_t)virtual) * sizeof(uint32_t)));

	mem_assert(*pte & PTE_PRESENT);

	*pte = 0;

	mem_tlb_flush((void *) virtual);

	FUNC_LEAVE();
}

static int clone_pt(void *source, void *dest)
{
	int i;
	uint32_t *src_pte = source;
	uint32_t *dest_pte = dest;
	addr_t phy_page;
	uint32_t flags;
	void *dest_virt_page;
	void *src_virt_page;
	assert((source) && (dest));

	FUNC_ENTER();

	for (i = 0; i < PAGE_TABLE_SIZE; i++) {
		/* Skip if nothing there */
		if (!(src_pte[i] & PTE_PRESENT))
			continue;
		else {
			if (src_pte[i] & PTE_TEMPORARY)
				continue;

			/* User page, copy that */
			phy_page = mem_get_page();

			if (!phy_page) {
				pr_error("Can't allocate page\r\n");
				return -1;
			}

			dest_virt_page = mem_page_map_kernel(phy_page, 1, READ_WRITE_KERNEL | PTE_TEMPORARY);

			if (!dest_virt_page) {
				pr_error("Can't allocate page\r\n");
				mem_free_page(phy_page);
				return -1;
			}

			src_virt_page = mem_page_map_kernel(src_pte[i] & PAGE_MASK, 1, READ_WRITE_KERNEL | PTE_TEMPORARY);

			if (!src_virt_page) {
				pr_error("Can't allocate page\r\n");
				mem_page_unmap(dest_virt_page);
				mem_free_page(phy_page);
				return -1;
			}

			memcpy(dest_virt_page, src_virt_page, PAGE_SIZE);

			/* Place it in the PDT */
			flags = PTE_PRESENT;

			if (src_pte[i] & PTE_USER_PAGE)
				flags |= PTE_USER_PAGE;

			if (src_pte[i] & PTE_ALLOW_WRITE)
				flags |= PTE_ALLOW_WRITE;

			dest_pte[i] = phy_page | flags;

			// Unmap the temporary pages
			mem_page_unmap(dest_virt_page);
			mem_page_unmap(src_virt_page);
		}
	}

	FUNC_LEAVE();
	return 0;

}
int clone_pdt(void *v_source, void *v_dest, addr_t p_dest)
{
	uint32_t i;
	int ret;
	uint32_t *src_pdt = v_source;
	uint32_t *dest_pdt = v_dest;
	addr_t phy_page;
	void *dest_virt_page;
	void *src_virt_page;
	uint32_t flags;
	FUNC_ENTER();

	assert((v_source) && (v_dest));


	for (i = 0; i < PAGE_DIRECTORY_SIZE - 1; i++) {
		/* Skip if nothing there */

		//pr_debug("i=%u src_pdt = %x\r\n", i, src_pdt[i]);

		if (!(src_pdt[i] & PDT_PRESENT))
			continue;
		else {
			if ((src_pdt[i] & PDT_HUGE_PAGE)) {
				//Huge pages are for kernel, we allow write on these
				dest_pdt[i] = (src_pdt[i] & PAGE_MASK) | PDT_PRESENT | PDT_ALLOW_WRITE | PDT_HUGE_PAGE;
			} else if ((!(src_pdt[i] & PDT_USER_PAGE))
			           && (i >= FRAME_TO_PDE_INDEX((addr_t) KERNEL_VIRTUAL_BASE)))
				dest_pdt[i] = (src_pdt[i] & PAGE_MASK) | PDT_PRESENT | PDT_ALLOW_WRITE;
			else {
				/* User page, copy that */
				phy_page = mem_get_page();

				if (!phy_page) {
					pr_error("Can't allocate page\r\n");
					return -1;
				}

				//Temporary map
				src_virt_page = mem_page_map_kernel(src_pdt[i] & PAGE_MASK, 1, READ_ONLY_KERNEL | PTE_TEMPORARY);

				if (!src_virt_page) {
					pr_error("Can't allocate page\r\n");
					mem_free_page(phy_page);
					return -1;
				}

				//Temporary map
				dest_virt_page = mem_page_map_kernel(phy_page, 1, READ_WRITE_KERNEL | PTE_TEMPORARY);

				if (!dest_virt_page) {
					pr_error("Can't allocate page\r\n");
					mem_page_unmap(src_virt_page);
					mem_free_page(phy_page);
					return -1;
				}

				memset(dest_virt_page, 0, PAGE_SIZE);

				/* Place it in the PDT */
				flags = PDT_PRESENT;

				if (src_pdt[i] & PDT_USER_PAGE)
					flags |= PDT_USER_PAGE;

				if (src_pdt[i] & PDT_ALLOW_WRITE)
					flags |= PDT_ALLOW_WRITE;

				dest_pdt[i] = phy_page | flags;

				ret = clone_pt(src_virt_page, dest_virt_page);


				// Unmap all the temp stuff
				mem_page_unmap(src_virt_page);
				mem_page_unmap(dest_virt_page);

				if (ret) {
					pr_error("page table clone failed\r\n");
					mem_free_page(phy_page);
					return -1;
				}
			}
		}
	}

	dest_pdt[1023] = p_dest | READ_WRITE_KERNEL | PDT_PRESENT;

	return 0;
}

void *mem_calloc_pdt(addr_t *p_addr)
{

	if (!p_addr) {
		pr_error("NULL ptr\r\n");
		return NULL;
	}

	*p_addr = mem_get_page();
	void *v_addr;

	if (!*p_addr) {
		pr_error("No more physical space\r\n");
		return NULL;
	}

	v_addr = mem_page_map_kernel(*p_addr, 1, READ_WRITE_USER);

	if (!v_addr) {
		pr_error("No more virtual space\r\n");
		mem_free_page(*p_addr);
		return NULL;
	}

	memset(v_addr, 0, PAGE_SIZE);
	pr_info("New PDT: physical: 0x%X, virtual: 0x%X\r\n", *p_addr, v_addr);
	return v_addr;
}

void mem_free_pdt(void *pdt)
{
// TODO: Implement
}
void mem_release_pdt(addr_t p_addr, void *v_addr)
{
	if ((!p_addr) || (!v_addr))
		pr_error("NULL ptr\r\n");

	mem_free_page(p_addr);
	mem_page_unmap(v_addr);
}





