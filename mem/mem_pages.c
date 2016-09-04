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

#include <mem/mem_pages.h>
#include <printk.h>
#include <OS_types.h>
#include <kernel/isr.h>
#include <lib/kmalloc.h>
#include <lib/OS_memory.h>
#include <kernel.h>

#define PAGE_SIZE 4096

typedef struct
{
	addr_t phys_start;
	addr_t phys_bitmap;
	uint32_t total_bits;
	uint32_t * bitmap;
} phys_memory;


// These are defined in the linker script, with these we calculate the size of the kernel on runtime.
extern uint32_t kernel_start;
extern uint32_t kernel_end;
extern uint32_t pdt;

static uint32_t total_memory = 0;
static uintptr_t physical_start = 0;
static uintptr_t pages_start;

static uint32_t total_pages;

static phys_memory physmem;


static uint32_t *kernel_pdt = &pdt;

#define KERNEL_VIRTUAL_BASE 0xC0000000

#define PHYSICAL_ALLOCATOR_BITMAP_BASE 0xC0000000

#define KERNEL_HEAP_VIR_START 0xD0000000
#define KERNEL_HEAP_VIR_END 0xE0000000
#define KERNEL_HEAP_SIZE (KERNEL_HEAP_VIR_END - KERNEL_HEAP_VIR_START)

#define PDE_MIRROR_BASE 0xFFC00000 
#define PDT_MIRROR_BASE 0xFFFFF000

#define BIT_CHECK(a,b) ((a) & (1<<(b)))
#define BIT_SET(a,b) ((a) |= (1<<(b)))
#define BIT_CLEAR(a,b) ((a) &= ~(1<<(b)))
#define IS_PAGE_ALIGNED(POINTER) \
	    (((uintptr_t)(const void *)(POINTER)) % (PAGE_SIZE) == 0)

#define PAGE_DIRECTORY_SIZE 1024
#define PAGE_TABLE_SIZE 1024

#define FRAME_TO_PDE_INDEX(_x) (_x >> 22)
#define FRAME_TO_PTE_INDEX(_x) ((_x << 10) >> 22) 


#define PAGE_ENTRY_PRESENT (1)
#define PAGE_ENTRY_WRITEABLE (1 << 1)
#define PAGE_ENTRY_USER (1 << 2)


#define VADDR_TO_PAGE_DIR (_x) ((_x ~0xFFF) / 0x400000)

static inline uint32_t divide_up(uint32_t value, uint32_t align)
{
	uint32_t ret;
	ret = value / align;
	if (value % align)
		ret++;
	return ret;
}

void page_fault_handler(registers_t regs);
void mem_heap_init();
void mem_phys_init(addr_t phy_start, uint32_t total_pages);

static inline void mem_tlb_flush(void* m)
{
    /* Clobber memory to avoid optimizer re-ordering access before invlpg, which may cause nasty bugs. */
	__asm volatile ( "invlpg (%0)" : : "b"(m) : "memory" );
}

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

	uint32_t reserved_pages;
	uint32_t kernel_size;
	uint32_t required_kernel_pages;
	uint32_t addr;


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
	addr_t mirror = (addr_t) (((char *) kernel_pdt) - KERNEL_VIRTUAL_BASE);
	// Map the page directory to itself.
	kernel_pdt[1023] = mirror |  3;

	mem_tlb_flush((void *) PDE_MIRROR_BASE);

	kernel_size = ((uint32_t) &kernel_end - (uint32_t) &kernel_start);
	required_kernel_pages = (kernel_size / PAGE_SIZE) + 1;

	printk("Kernel start: 0x%x, kernel end: 0x%x\r\n", (uint32_t) &kernel_start, &kernel_end);
	printk("Kernel occupies 0x%x bytes, consuming %u pages\r\n", kernel_size, required_kernel_pages);


	// ugly hack
	//
	physical_start = ((uint32_t) &kernel_start) - KERNEL_VIRTUAL_BASE;

	physical_start += required_kernel_pages * PAGE_SIZE;
	total_memory -= required_kernel_pages * PAGE_SIZE;


	mem_phys_init(physical_start, total_memory);
	while(1);
//	mem_heap_init();
}

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


void mem_phys_init(addr_t phy_start, uint32_t total_memory)
{
	uint32_t total_pages = total_memory / PAGE_SIZE;
	uint32_t required_bytes = total_pages / 8;
	uint32_t pte_count = divide_up(required_bytes, PAGE_SIZE);
	uint32_t pde_count = divide_up(pte_count, 1024);
	addr_t page, vaddr;
	uint32_t i,j;

	void * access_ptr;	
	uint8_t * pde_mirror = (uint8_t *) PDE_MIRROR_BASE;

	physmem.phys_bitmap =  phy_start;
	printk("Physical memory zone set to: 0x%x size: 0x%x\r\n", physical_start, total_memory);
	printk("Number of pages: %u Required Bytes: %u\r\n", total_pages, required_bytes);

	printk("Required PTE: %u, Required PDE: %u\r\n", pte_count, pde_count);


/* We're placing our physical manager just after the kernel */
	vaddr = PHYSICAL_ALLOCATOR_BITMAP_BASE;

	for(i= 0; i < pde_count; i++)
	{
		// Get PTE page
		page = (addr_t) physical_start;
		physical_start += PAGE_SIZE;
		total_memory -= PAGE_SIZE;

		// Put it in PDT
		kernel_pdt[FRAME_TO_PDE_INDEX(vaddr)] = page | 3;
		// Invalidate cache
		access_ptr = pde_mirror + (FRAME_TO_PDE_INDEX(vaddr) * 0x1000);
		mem_tlb_flush(PDT_MIRROR_BASE);
		// Clear the PDE table
		memset(access_ptr, 0, PAGE_SIZE);
	
		for (j = 0; j < MIN(pte_count, 1024) ; j++)
		{
			page = (addr_t) physical_start;
			physical_start += PAGE_SIZE;
			total_memory -= PAGE_SIZE;

			*(uint32_t *) (access_ptr + (j * sizeof(uint32_t))) = page | 3;

		}
		if (j == 1024)
		{
			pte_count -= 1024;
		}

	}

	/* Invalidate all our memory region */

	for (i = 0, access_ptr = PHYSICAL_ALLOCATOR_BITMAP_BASE;  i < pte_count ; i++, access_ptr += PAGE_SIZE)
	{
		mem_tlb_flush(access_ptr);	
	}

	memset( (void *)PHYSICAL_ALLOCATOR_BITMAP_BASE, 0, required_bytes);
}

void *mem_page_map(addr_t page)
{

}


void mem_page_unmap(addr_t page)
{

}


void mem_page_free(addr_t page)
{
	int entry;
	int bit;

	// Sanity, check that the address given is page aligned.
	if (! IS_PAGE_ALIGNED(page))
	{
		printk("recieved unaligned page to free, that's a problem :(\r\n");
		panic();
	}

	page -= physical_start;
	entry = page / PAGE_SIZE;
	bit = page % PAGE_SIZE;

	// Sanity, check that the address given was actually alocated.
	if (! BIT_CHECK(physmem.bitmap[entry], bit))
	{
		printk("Trying to free an already free page, that's a problem :(\r\n");
		panic();
	}

	BIT_CLEAR(physmem.bitmap[entry], bit);


}

addr_t mem_page_get()
{
	uint32_t i = 0;
	uint32_t j = 0;
	bool found = false;

	for (i = 0; i < total_pages; i++)
	{
		if (physmem.bitmap[i])
		{
			// There's at least one free page in this int bitmap, find it and mark it.
			//
			//
			for (j = 0; j < 32; j++)
			{
				if (! BIT_CHECK(physmem.bitmap[i], j))
				{
					BIT_SET(physmem.bitmap[i], j);
					found = true;
					break;
				}
			}

			printk("ERROR:  internal error\r\n");
			panic();
		}

	}

	if (found)
	{
		return (addr_t)((i * 32 + j) * PAGE_SIZE) + physical_start;
	}
	else
	{
		return NULL;
	}
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

void mem_heap_init()
{
	uint32_t pte_count = KERNEL_HEAP_SIZE / PAGE_SIZE / 1024;
	uint32_t pde_count = divide_up(pte_count, 1024);
	addr_t page;
	addr_t vaddr =  KERNEL_HEAP_VIR_START;
	char * access_ptr;
	uint8_t * pde_mirror = (uint8_t *) PDE_MIRROR_BASE;
	
	uint32_t i, j;

	printk("mem_heap_init: pte_count = 0x%x pde_count = 0x%x\r\n", pte_count, pde_count);

	for(i= 0; i < pde_count; i++)
	{
		// Get PTE page
		page = mem_page_get();
		// Put it in PDT
		kernel_pdt[FRAME_TO_PTE_INDEX(vaddr)] = page;
		// Invalidate cache
		mem_tlb_flush(pde_mirror + (i * PAGE_SIZE));	
		// Clear the PDE table
		memset(pde_mirror + (i * PAGE_SIZE), 0, PAGE_SIZE);

		for (j = 0; j < MIN(pte_count, 1024) ; j++)
		{
			page = mem_page_get();
			*(uint32_t *) (pde_mirror + (j * sizeof(uint32_t))) = page;
			access_ptr = (char *) vaddr;
			memset(access_ptr, 0, PAGE_SIZE);
			mem_tlb_flush((void *) KERNEL_HEAP_VIR_START + (i * PAGE_SIZE));
			vaddr += PAGE_SIZE;
		}
		if (j == 1024)
		{
			pte_count -= 1024;
		}

	}

}
/* HEAP Handling goes here */

int mem_heap_lock()
{
	//todo: implement when necessary
	return 0;
}

int mem_heap_unlock()
{
	//todo: implement when necessary
	return 0;
}

void *mem_heap_alloc(int count)
{
#if 0
	static int first = 0;

	addr_t pos = KERNEL_HEAP_VIR_START;

	int pte_index;
	int pde_index;

	while (pos < KERNEL_HEAP_VIR_END)
	{
		/* Find a free spot */
		pde_index = FRAME_TO_PTE_INDEX(pos);
		pte_index = FRAME_TO_PTE_INDEX(pos);

		if (kernel_pdt[pte_index] & PAGE_ENTRY_PRESENT)
		{
			if (
		}	

	
	}	
	if (!first)
	{
#endif	
	//todo: implement
	return NULL;
}

int mem_heap_free(void *addr , int count)
{
	//todo: implement
	return 0;
}

