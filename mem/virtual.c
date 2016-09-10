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

#include <mem/memory.h>
#include <printk.h>
#include <types.h>
#include <kernel/isr.h>
#include <lib/kmalloc.h>
#include <lib/string.h>
#include <kernel.h>
#include <kernel/bits.h>

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



//static  uint32_t * kernel_pdt = (uint32_t *) PDT_MIRROR_BASE;
extern uint32_t pdt;
//TODO: Once we enable mirror in assembly, we can drop this crap.
static uint32_t *kernel_pdt = &pdt;
// These are defined in the linker script, with these we calculate the size of the kernel on runtime.
extern uint32_t kernel_end, kernel_start;

void page_fault_handler(registers_t regs);
void mem_heap_init();
void mem_phys_init(addr_t phy_start, uint32_t total_pages);


#if 0
void mem_switch_page_directory(addr_t new_dir)
{
	void *m = 0;
	__asm volatile("mov %0, %%cr3":: "r"(new_dir));
	__asm volatile("invlpg (%0)" : : "b"(m) : "memory") ;
//	uint32_t cr0;
//	asm volatile("mov %%cr0, %0": "=r"(cr0));
//	cr0 |= 0x80000000; // Enable paging!
//	asm volatile("mov %0, %%cr0":: "r"(cr0));
}
#endif

void mem_init(multiboot_info_t *mbi)
{
	uint32_t total_memory;
	uint32_t physical_start;
	uint32_t kernel_size;
	uint32_t required_kernel_pages;
	uint32_t addr;

	FUNC_ENTER();
	register_interrupt_handler(14, page_fault_handler);

	printk("Memory init:\r\ndetecting physical memory.\r\n");

	multiboot_memory_map_t *mmap = (multiboot_memory_map_t *)(mbi->mmap_addr + KERNEL_VIRTUAL_BASE);

	while (mmap < mbi->mmap_addr + KERNEL_VIRTUAL_BASE +  mbi->mmap_length)
	{
		mmap = (multiboot_memory_map_t *)((unsigned int)mmap + mmap->size + sizeof(unsigned int));

		if (mmap->size == 0)
		{
			// That's the end of the list
			break;
		}

		printk("Memory region size: %u address: 0x%llx length: 0x%llx type: %u\r\n",
		       mmap->size, mmap->addr, mmap->len, mmap->type);

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
		printk("No physical memory!\r\n");
		panic();
	}

	//TODO: Move to assembly. doesn't belong here.
	addr_t mirror = (addr_t)(((char *) kernel_pdt) - KERNEL_VIRTUAL_BASE);
	// Map the page directory to itself.
	kernel_pdt[1023] = mirror | 3;

	mem_tlb_flush((void *) PDE_MIRROR_BASE);

	kernel_size = ((uint32_t) &kernel_end - (uint32_t) &kernel_start);
	required_kernel_pages = (kernel_size / PAGE_SIZE) + 1;

	printk("Kernel start: 0x%x, kernel end: 0x%x\r\n", (uint32_t) &kernel_start, &kernel_end);
	printk("Kernel occupies 0x%x bytes, consuming %u pages\r\n", kernel_size, required_kernel_pages);

	// TODO: ugly hack
	physical_start = ((uint32_t) &kernel_start) - KERNEL_VIRTUAL_BASE;

	physical_start += required_kernel_pages * PAGE_SIZE;
	total_memory -= required_kernel_pages * PAGE_SIZE;


	mem_phys_init(physical_start, total_memory);

	mem_heap_init();

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

	panic();
}



int mem_map_con_pages(addr_t physical, uint32_t count, addr_t virtual)
{
	uint32_t pte_count = count;
	uint32_t pde_count = divide_up(pte_count, 1024);
	addr_t page;
	char *access_ptr ;
	uint32_t i, j;

	FUNC_ENTER();

	uint32_t *pte;
	printk("mem_map_con_pages: pte_count = 0x%x pde_count = 0x%x\r\n", pte_count, pde_count);

	for (i = 0; i < pde_count; i++)
	{
		// Get PDE page
		printk("i = %u\r\n", i);
		access_ptr = (char *)(PDE_MIRROR_BASE + (FRAME_TO_PDE_INDEX(virtual) * 0x1000));

		printk("kernel pdt entry: %u, real one: %u,  0x%x\r\n", FRAME_TO_PDE_INDEX(virtual), VADDR_TO_PAGE_DIR(virtual),  kernel_pdt[FRAME_TO_PDE_INDEX(virtual)]);

		if (!(kernel_pdt[FRAME_TO_PDE_INDEX(virtual)] & 3))
		{
			page = mem_get_page();

			assert(page != 0);

			// Put it in PDT
			kernel_pdt[FRAME_TO_PDE_INDEX(virtual)] = page | 3;
			// Invalidate cache
			mem_tlb_flush(access_ptr);
			// Clear the PDE table
			memset(access_ptr, 0, PAGE_SIZE);
		}

		for (j = 0; j < MIN(pte_count, 1024) ; j++)
		{
			pte = (uint32_t *)(access_ptr + (j * sizeof(uint32_t)));

//			printk("PTE is = 0x%x, access_ptr = 0x%x\r\n", pte, access_ptr);


			assert(!(*pte & 3)); // the PDE is already mapped. that's a bug.

			page = physical + (((i * 1024) + j) * PAGE_SIZE);

			*pte = page | 3;

			mem_tlb_flush((void *) KERNEL_HEAP_VIR_START + (((i * 1024) + j) * PAGE_SIZE));
//			printk("flushing address: %x\r\n", KERNEL_HEAP_VIR_START + (((i * 1024) + j) * PAGE_SIZE));
			virtual += PAGE_SIZE;
		}

		if (j == 1024)
		{
			pte_count -= 1024;
		}

	}

	FUNC_LEAVE();
	return 0;
}

int mem_unmap_con_pages(addr_t virtual, uint32_t count)
{
	FUNC_ENTER();
	FUNC_LEAVE();
	return 0;
}


