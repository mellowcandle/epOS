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
#include <lib/string.h>
#include <kernel.h>
#include <kernel/bits.h>


extern uint32_t kernel_start;
extern uint32_t kernel_end;

#define PAGE_TO_BITMAP_INDEX(_x) (_x / 32)
#define PAGE_TO_BITMAP_OFFSET(_x) (_x % 32)

typedef struct
{
	addr_t phys_start;
	addr_t phys_bitmap;
	uint32_t total_pages;
	uint32_t *bitmap;
} phys_memory;


static phys_memory physmem;

void mem_phys_init(addr_t phy_start, uint32_t total_memory)
{
	physmem.total_pages = total_memory / PAGE_SIZE;
	uint32_t required_bytes = physmem.total_pages / 8;
	uint32_t pte_count = divide_up(required_bytes, PAGE_SIZE);
	uint32_t pde_count = divide_up(pte_count, 1024);
	addr_t page, vaddr;
	uint32_t i, j;

	uint32_t *kernel_pdt = (void *) PDT_MIRROR_BASE;
	void *access_ptr;
	uint8_t *pde_mirror = (uint8_t *) PDE_MIRROR_BASE;

	physmem.phys_start = physmem.phys_bitmap = phy_start;
	printk("Physical memory zone set to: 0x%x size: 0x%x\r\n", physmem.phys_start, total_memory);
	printk("Number of pages: %u Required Bytes: %u\r\n", physmem.total_pages, required_bytes);

	printk("Required PTE: %u, Required PDE: %u\r\n", pte_count, pde_count);


	/* We're placing our physical manager just after the kernel */
	vaddr = PHYSICAL_ALLOCATOR_BITMAP_BASE;

	for (i = 0; i < pde_count; i++)
	{
		// Get PTE page
		page = (addr_t) physmem.phys_start;
		physmem.phys_start += PAGE_SIZE;
		total_memory -= PAGE_SIZE;

		// Put it in PDT
		kernel_pdt[FRAME_TO_PDE_INDEX(vaddr)] = page | 3;
		// Invalidate cache
		access_ptr = pde_mirror + (FRAME_TO_PDE_INDEX(vaddr) * 0x1000);
		mem_tlb_flush((void *) PDT_MIRROR_BASE);
		// Clear the PDE table
		memset(access_ptr, 0, PAGE_SIZE);


		for (j = 0; j < MIN(pte_count, 1024) ; j++)
		{
			page = (addr_t) physmem.phys_start;
			physmem.phys_start += PAGE_SIZE;
			total_memory -= PAGE_SIZE;

			*(uint32_t *)(access_ptr + (j * sizeof(uint32_t))) = page | 3;

		}

		if (j == 1024)
		{
			pte_count -= 1024;
		}

	}

	/* Invalidate all our memory region */

	for (i = 0, access_ptr = (void *) PHYSICAL_ALLOCATOR_BITMAP_BASE;  i < pte_count ; i++, access_ptr += PAGE_SIZE)
	{
		mem_tlb_flush(access_ptr);
	}

	memset((void *)PHYSICAL_ALLOCATOR_BITMAP_BASE, 0, required_bytes);

}




void mem_free_pages(addr_t addr, uint32_t count)
{
	uint32_t i,j;

	if (!IS_PAGE_ALIGNED(addr))
	{
		printk("mem_free_pages: addr: 0x%x is not page aligned\r\n", addr);
		panic();
	}
	uint32_t page_num = ((char *) addr - (char *) physmem.phys_start) / PAGE_SIZE;
		
	for (i = PAGE_TO_BITMAP_INDEX(page_num), j = PAGE_TO_BITMAP_OFFSET(page_num); count > 0; count--)
	{
		BIT_CLEAR(physmem.bitmap[i],j);
		j++;
		if (j == 32)
		{
			i++;
			j = 0;
		}
	}

}

void mem_free_page(addr_t addr)
{
	mem_free_pages(addr, 1);
}

addr_t mem_get_pages(uint32_t count)
{
	uint32_t i = 0;
	uint32_t j = 0;
	uint32_t page = 0;
	uint32_t found = 0;

	while (page < physmem.total_pages)
	{
		// 32 pages are full, skip it.
		if (physmem.bitmap[PAGE_TO_BITMAP_INDEX(page)] == 0xFFFFFFFF)
		{
			page += 32;
			continue;
		}
		else
		{
			// Find the right spot

	}

}

addr_t mem_get_page()
{
	return mem_get_pages(1);
}

uint32_t _mem_bitmap_get_next_free_page(uint32_t from)
{
	uint32_t i = PAGE_TO_BITMAP_INDEX(from);
	uint32_t j = PAGE_TO_BITMAP_OFFSET(from);
	bool found = false;

	for (;from < physmem.total_pages; i++);
}

#if 0
addr_t mem_page_get()
{
	uint32_t i = 0;
	uint32_t j = 0;
	bool found = false;

	for (i = 0; i < physmem.total_pages; i++)
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
		return (addr_t)((i * 32 + j) * PAGE_SIZE) + physmem.phys_start;
	}
	else
	{
		return NULL;
	}
}
#endif
