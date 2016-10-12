/*
	This is free and unencumbered software released into the public domain.

	Anyone is free to copy, modify, publish, use, compile, sell, or
	distribute this software, either in source code form or as a compiled
	binary, for any purpose, commercial or non-commercial, and by any
	means.

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
#include <printk.h>
#include <types.h>
#include <isr.h>
#include <lib/kmalloc.h>
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


#define PDT_PRESENT			BIT(0)
#define PDT_ALLOW_WRITE		BIT(1)
#define PDT_USER_PAGE		BIT(2)
#define PDT_PWD				BIT(3)
#define PDT_PCD				BIT(4)
#define PDT_ACCESSED		BIT(5)
#define PDT_HUGE_PAGE		BIT(7)

#define PTE_PRESENT			BIT(0)
#define PTE_ALLOW_WRITE		BIT(1)
#define PTE_USER_PAGE		BIT(2)
#define PTE_PWT				BIT(3)
#define PTE_PCD				BIT(4)
#define PTE_ACCESSED		BIT(5)
#define PTE_DIRTY			BIT(6)
#define PTE_PAT				BIT(7)
#define PTE_GLOBAL			BIT(7)

static uint32_t *current_pdt = (uint32_t *) PDT_MIRROR_BASE;

addr_t virt_to_phys(void *addr)
{
	uint32_t *pte;
	uint32_t pde_idx = FRAME_TO_PDE_INDEX((addr_t) addr);
	uint32_t pte_idx = FRAME_TO_PTE_INDEX((addr_t) addr);

	assert(IS_PAGE_ALIGNED(addr));

	if (current_pdt[pde_idx] & PDT_PRESENT)
	{
		if (current_pdt[pde_idx] & PDT_HUGE_PAGE)
		{
			return (current_pdt[pde_idx] & PAGE_MASK) + ((addr_t)addr & HUGE_PAGE_MASK);
		}
		else
		{
			pte = (uint32_t *)(PDE_MIRROR_BASE + (pde_idx * 0x1000));
			pte += pte_idx; // int pointer aritmethics here.
			return (*pte & PAGE_MASK);
		}
	}

	return 0;
}



// These are defined in the linker script, with these we calculate the size of the kernel on runtime.
extern uint32_t kernel_end, kernel_start;

void page_fault_handler(registers_t regs);
void mem_heap_init();
void mem_phys_init(addr_t phy_start, uint32_t total_pages);
static addr_t mem_find_kernel_place(int request);


void mem_switch_page_directory(addr_t new_dir)
{
	void *m = 0;
	__asm volatile("mov %0, %%cr3":: "r"(new_dir));
	__asm volatile("invlpg (%0)" : : "b"(m) : "memory") ;
}

void *mem_calloc_pdt()
{
	addr_t p_addr = mem_get_page();
	void *v_addr;

	if (!p_addr)
	{
		pr_error("No more physical space\r\n");
		return NULL;
	}

	v_addr = mem_page_map_kernel(p_addr, 1, 0);

	if (!v_addr)
	{
		pr_error("No more virtual space\r\n");
		mem_free_page(p_addr);
		return NULL;
	}

	memset(v_addr, 0, PAGE_SIZE);

	return v_addr;
}

void dump_pdt()
{
	char *ptr;
	addr_t virtual_pos;

	for (int i = 0; i < PAGE_DIRECTORY_SIZE; i++)
	{
		if (current_pdt[i] & PDT_PRESENT) // exists
		{
			printk("PDE: %u value: 0x%x\r\n", i, current_pdt[i]);

			if (current_pdt[i] & PDT_HUGE_PAGE)
			{
				printk("Maps 4MB page from 0x%x to 0x%x\r\n", PDE_INDEX_TO_ADDR(i), PDE_INDEX_TO_ADDR(i + 1));
			}
			else
			{
				ptr = (char *) PDE_MIRROR_BASE + (i * 0x1000);

				for (int j = 0; j < PAGE_TABLE_SIZE; j++)
				{
					if (* ((uint32_t *)ptr) & PTE_PRESENT)
					{
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
void *mem_page_map_kernel(addr_t physical, int count, int flags)
{
	addr_t addr = mem_find_kernel_place(count);

	if (addr)
	{
		for (int i = 0 ; i < count; i++)
		{
			mem_page_map(physical + (i * PAGE_SIZE), addr + (i * PAGE_SIZE), flags);
		}

		return (void *) addr;
	}

	pr_error("No more virtual kernel space to satisfy request\r\n");

	while (1);

	panic();
	return NULL;
}

static addr_t mem_find_kernel_place(int count)
{
	char *ptr;
	addr_t addr;
	int n = 0;
	bool begin = false;
	uint32_t start_i, start_j;

	/*  Algorithm:
	 *  This is pretty easy. we search the PDE & PDT for the first matching "request" pages.
	 *  When we find the first empty page, we mark the pos in start_i and start_j and set begin to true.
	 *  If we manage to count N consecutive pages we return the address sucessfuly */

	for (uint32_t i = FRAME_TO_PDE_INDEX(KERNEL_VIRTUAL_BASE); i < PAGE_DIRECTORY_SIZE; i++)
	{
		if (current_pdt[i] & PDT_PRESENT) // exists
		{
			if (current_pdt[i] & PDT_HUGE_PAGE)
			{
				if (begin)
				{
					begin = false;
					n = 0;
				}

				continue;
			}
			else
			{
				ptr = (char *) PDE_MIRROR_BASE + (i * 0x1000);

				for (uint32_t j = 0; j < PAGE_TABLE_SIZE; j++, ptr += 4)
				{
					if (* ((uint32_t *)ptr) & PTE_PRESENT)
					{
						if (begin)
						{
							begin = false;
							n = 0;
						}

						continue;
					}
					else
					{
						if (!begin)
						{
							start_i = i;
							start_j = j;
							begin = true;
						}

						if (++n >= count)
						{
							break;
						}
					}
				}

			}
		}
		else
		{
			// PDE is not maped in, this means we have 1024 empty entries in here.
			if (!begin)
			{
				start_i = i;
				begin = true;
				start_j = 0;
			}

			n += 1024;
		}

		if (n >= count)
		{
			addr = PDE_INDEX_TO_ADDR(start_i) + (start_j * PAGE_SIZE);
			pr_debug("Found place for %u pages starting from 0x%x!\r\n", count, addr);
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

	extern heap_t kernel_heap;
	FUNC_ENTER();
	register_interrupt_handler(14, page_fault_handler);

	pr_info("Memory init:\r\ndetecting physical memory.\r\n");

	multiboot_memory_map_t *mmap = (multiboot_memory_map_t *)(mbi->mmap_addr + KERNEL_VIRTUAL_BASE);

	while ((uint32_t) mmap < (mbi->mmap_addr + KERNEL_VIRTUAL_BASE +  mbi->mmap_length))
	{
		mmap = (multiboot_memory_map_t *)((unsigned int)mmap + mmap->size + sizeof(unsigned int));

		if (mmap->size == 0)
		{
			// That's the end of the list
			break;
		}

		pr_debug("Memory region address: 0x%llx length: 0x%llx type: %u\r\n",
		         mmap->addr, mmap->len, mmap->type);

		// We only support one zone, we'll take the biggest.
		//
		if (mmap->type == 1)
		{
			if (mmap->len > total_memory)
			{
				total_memory = mmap->len;
				physical_start = mmap->addr;
			}
		}

	}

	if (total_memory == 0)
	{
		pr_fatal("No physical memory!\r\n");
		panic();
	}

	kernel_size = ((uint32_t) &kernel_end - (uint32_t) &kernel_start);
	required_kernel_pages = (kernel_size / PAGE_SIZE) + 1;

	pr_debug("Kernel start: 0x%x, kernel end: 0x%x\r\n", (uint32_t) &kernel_start, (uint32_t) &kernel_end);
	pr_debug("Kernel occupies 0x%x bytes, consuming %u pages\r\n", kernel_size, required_kernel_pages);

	physical_start = ((uint32_t) &kernel_start) - KERNEL_VIRTUAL_BASE;
	physical_start += required_kernel_pages * PAGE_SIZE;
	physical_start = PAGE_ALIGN_UP(physical_start);
	total_memory -= required_kernel_pages * PAGE_SIZE;

	mem_phys_init(physical_start, total_memory);

	mem_heap_init(get_kernel_heap(), KERNEL_HEAP_VIR_START, KERNEL_HEAP_SIZE);

	FUNC_LEAVE();
}



void page_fault_handler(registers_t regs)
{

	addr_t cr2, err_code = regs.err_code;

	bool page_present; // else missing
	bool page_read;  // else write
	bool page_user;  // else kernel

	__asm__ volatile("mov %%cr2, %0" : "=r"(cr2));

	if (err_code & 1)
	{
		page_present = true;
	}
	else
	{
		page_present = false;
	}

	if (err_code & (1 << 1))
	{
		page_read = false;
	}
	else
	{
		page_read = true;
	}

	if (err_code & (1 << 2))
	{
		page_user = true;
	}
	else
	{
		page_user = false;
	}


	printk("Page fault: linear address: 0x%x\r\n", cr2);
	printk("Details: Page %s, ", page_present ? "present" : "not present");
	printk("Ocurred on %s, ", page_read ? "read" : "write");
	printk("Page ownership %s\r\n", page_user ? "user" : "kernel");

	irq_reg_dump(&regs);
	dump_pdt();
	panic();
}

int mem_page_map(addr_t physical, addr_t virtual, int flags)
{

	addr_t page;
	char *access_ptr;
	uint32_t *pte;

	pr_debug("+mem_page_map physical: 0x%x virtual 0x%x flags %X\r\n", physical, virtual, flags);

	assert(IS_PAGE_ALIGNED(physical));

	access_ptr = (char *)(PDE_MIRROR_BASE + (FRAME_TO_PDE_INDEX(virtual) * 0x1000));


	// Check if the PDT exists
	if (!(current_pdt[FRAME_TO_PDE_INDEX(virtual)] & PDT_PRESENT))
	{
		pr_debug("mem_map_page: PDT missing, creating and mapping\r\n");
		page = mem_get_page();

		mem_assert(page != 0);

		// Put it in PDT
		current_pdt[FRAME_TO_PDE_INDEX(virtual)] = page | PDT_PRESENT | PDT_ALLOW_WRITE;
		// Invalidate cache
		mem_tlb_flush(access_ptr);
		// Clear the PDE table
		memset(access_ptr, 0, PAGE_SIZE);
	}

	// Insert the PTE.
	pte = (uint32_t *)(access_ptr + (FRAME_TO_PTE_INDEX(virtual) * sizeof(uint32_t)));

//	mem_assert(!(*pte & 3));
	if (*pte & PTE_PRESENT)
	{
		printk("assert failed: ");
		pr_error("+mem_page_map physical: 0x%x virtual 0x%x flags %X\r\n", physical, virtual, flags);

//		dump_pdt();
		while (1);
	}

	*pte = physical | PTE_PRESENT | PTE_ALLOW_WRITE;

	mem_tlb_flush((void *) virtual);

	FUNC_LEAVE();
	return 0;
}


void mem_page_unmap(addr_t virtual)
{
	char *access_ptr;
	uint32_t *pte;

	//TODO: unmap page directory if necessary
	FUNC_ENTER();

	access_ptr = (char *)(PDE_MIRROR_BASE + (FRAME_TO_PDE_INDEX(virtual) * 0x1000));

	pte = (uint32_t *)(access_ptr + (FRAME_TO_PTE_INDEX(virtual) * sizeof(uint32_t)));

	mem_assert(*pte & PTE_PRESENT);

	*pte = 0;

	mem_tlb_flush((void *) virtual);

	FUNC_LEAVE();
}


