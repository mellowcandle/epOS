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


#ifndef MEM_PAGES_H_HGKLOSQ7
#define MEM_PAGES_H_HGKLOSQ7

#include <types.h>
#include <boot/multiboot.h>

#define PAGE_SIZE 4096
#define PAGE_DIRECTORY_SIZE 1024
#define PAGE_TABLE_SIZE 1024

#define IS_PAGE_ALIGNED(POINTER) \
	    (((uintptr_t)(const void *)(POINTER)) % (PAGE_SIZE) == 0)

#define FRAME_TO_PDE_INDEX(_x) (_x >> 22)
#define FRAME_TO_PTE_INDEX(_x) ((_x << 10) >> 22)
#define VADDR_TO_PAGE_DIR(_x) ((_x & ~0xFFF) / 0x400000)

#define PDE_MIRROR_BASE 0xFFC00000
#define PDT_MIRROR_BASE 0xFFFFF000

#define KERNEL_VIRTUAL_BASE 0xC0000000

#define PHYSICAL_ALLOCATOR_BITMAP_BASE 0xD0000000

#define KERNEL_HEAP_VIR_START 0xE0000000
#define KERNEL_HEAP_VIR_END 0xF0000000
#define KERNEL_HEAP_SIZE (KERNEL_HEAP_VIR_END - KERNEL_HEAP_VIR_START)

#define PAGE_ENTRY_PRESENT (1)
#define PAGE_ENTRY_WRITEABLE (1 << 1)
#define PAGE_ENTRY_USER (1 << 2)

typedef uintptr_t addr_t;

extern uint32_t pdt;

static inline void mem_tlb_flush(void *m)
{
	/* Clobber memory to avoid optimizer re-ordering access before invlpg, which may cause nasty bugs. */
	__asm volatile("invlpg (%0)" : : "b"(m) : "memory");
}


void mem_init(multiboot_info_t *mbi);
addr_t mem_get_pages(uint32_t count);
void mem_free_pages(addr_t addr, uint32_t count);
int mem_map_con_pages(addr_t physical, uint32_t count, addr_t virtual);
int mem_unmap_con_pages(addr_t virtual, uint32_t count);

static inline void  mem_free_page(addr_t addr)
{
	mem_free_pages(addr, 1);
}

static inline addr_t mem_get_page()
{
	return mem_get_pages(1);
}

int mem_page_map(addr_t physical, addr_t virtual, int flags);
int mem_page_unmap(addr_t virtual);



#endif /* end of include guard: MEM_PAGES_H_HGKLOSQ7 */

