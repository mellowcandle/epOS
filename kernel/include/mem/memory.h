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
#include <bits.h>

#define PAGE_SIZE 4096
#define PAGE_MASK (~(PAGE_SIZE-1))

#define HUGE_PAGE_SIZE (PAGE_SIZE * 1024)
#define HUGE_PAGE_MASK (~(HUGE_PAGE_SIZE-1))

#define PAGE_DIRECTORY_SIZE 1024
#define PAGE_TABLE_SIZE 1024

#define PTE_ADDR_MASK BF_MASK(12,20)
#define PDE_ADDR_MASK BF_MASK(22,10)

#define IS_PAGE_ALIGNED(POINTER) \
	    (((uintptr_t)(const void *)(POINTER)) % (PAGE_SIZE) == 0)

#define PAGE_ALIGN_DOWN(_x) (_x & PAGE_MASK)
#define PAGE_ALIGN_UP(_x) (((_x)+PAGE_SIZE - 1) & PAGE_MASK)

#define FRAME_TO_PDE_INDEX(_x) (_x >> 22)
#define FRAME_TO_PTE_INDEX(_x) ((_x << 10) >> 22)
#define VADDR_TO_PAGE_DIR(_x) ((_x & ~0xFFF) / 0x400000)

#define PDE_INDEX_TO_ADDR(_x) ((_x) << 22)

#define PDE_MIRROR_BASE 0xFFC00000
#define PDT_MIRROR_BASE 0xFFFFF000

#define KERNEL_VIRTUAL_BASE 0xC0000000

#define PHYSICAL_ALLOCATOR_BITMAP_BASE 0xD0000000

#define KERNEL_HEAP_VIR_START 0xD2000000
#define KERNEL_HEAP_VIR_END 0xE0000000
#define KERNEL_HEAP_SIZE (KERNEL_HEAP_VIR_END - KERNEL_HEAP_VIR_START)

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

#define READ_ONLY_USER (PTE_USER_PAGE  | PTE_PRESENT)
#define READ_WRITE_USER (PTE_USER_PAGE | PTE_ALLOW_WRITE | PTE_PRESENT)
#define READ_ONLY_KERNEL (PTE_PRESENT)
#define READ_WRITE_KERNEL (PTE_ALLOW_WRITE | PTE_PRESENT)


extern uint32_t pdt;

typedef struct
{

	uint32_t total_pages;
	uint32_t used_pages;
	addr_t   location;
	bool initalized;
} heap_t;

static inline void mem_tlb_flush(void *m)
{
	/* Clobber memory to avoid optimizer re-ordering access before invlpg, which may cause nasty bugs. */
	__asm volatile("invlpg (%0)" : : "b"(m) : "memory");
}


void mem_init(multiboot_info_t *mbi);
addr_t mem_get_pages(uint32_t count);
void mem_free_pages(addr_t addr, uint32_t count);
int mem_unmap_con_pages(addr_t virtual, uint32_t count);

static inline void  mem_free_page(addr_t addr)
{
	mem_free_pages(addr, 1);
}

static inline addr_t mem_get_page()
{
	return mem_get_pages(1);
}

int mem_page_map(addr_t physical, void *virtual, int flags);
void *mem_page_map_kernel(addr_t physical, int count, int flags);

void mem_page_unmap(void *virtual);

static inline void mem_page_unmap_multiple(void *virtual, int count)
{
	for (int i = 0; i < count; i++)
	{
		mem_page_unmap(virtual + (i * PAGE_SIZE));
	}
}

static inline int mem_identity_map(addr_t addr, int flags)
{
	return mem_page_map(addr, (void *)addr, flags);
}

static inline int mem_identity_map_multiple(addr_t addr, int flags, int count)
{
	for (int i = 0 ; i < count; i++)
	{
		mem_page_map(addr + (i * PAGE_SIZE), (void *) addr + (i * PAGE_SIZE), flags);
	}

	return 0;
}

static inline int mem_map_con_pages(addr_t physical, uint32_t count, addr_t virtual, int flags)
{
	for (uint32_t i = 0 ; i < count; i++)
	{
		mem_page_map(physical + (i * PAGE_SIZE), (void *)virtual + (i * PAGE_SIZE), flags);
	}

	return 0;

}

static inline void *mem_page_map_kernel_single(addr_t physical, int flags)
{
	return mem_page_map_kernel(physical, 1, flags);
}

/* Heap Management */
void mem_heap_init(heap_t *heap, addr_t vir_start, size_t size);
void mem_heap_destroy(heap_t *heap);
int mem_heap_lock(heap_t *heap);
int mem_heap_unlock(heap_t *heap);
void *_mem_heap_map_alloc(heap_t *heap, size_t count, addr_t hw_pages, bool hw);

static inline void *mem_heap_alloc(heap_t *heap, size_t count)
{
	return _mem_heap_map_alloc(heap, count, 0, false);
}

static inline void *mem_heap_map(heap_t *heap, size_t count, addr_t hw_pages)
{
	return _mem_heap_map_alloc(heap, count, hw_pages, true);
}

int mem_heap_free(heap_t *heap, void *addr , int count);
addr_t virt_to_phys(void *addr);

heap_t *get_kernel_heap();
void *mem_calloc_pdt(addr_t *p_addr);
int clone_pdt(void *v_source, void *v_dest, addr_t p_dest);

#endif /* end of include guard: MEM_PAGES_H_HGKLOSQ7 */

