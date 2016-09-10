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

#include <kernel.h>
#include <types.h>
#include <mem/memory.h>
#include <printk.h>

void mem_heap_init()
{
	addr_t physical;
	uint32_t count = KERNEL_HEAP_SIZE / PAGE_SIZE;

	physical = mem_get_pages(count);
	if (!physical)
		goto error;

	if (mem_map_con_pages(physical, count, KERNEL_HEAP_VIR_START) != 0)
		goto error;

	return;

error:
	printk("Can't allocate kernel HEAP\r\n");
	panic();

}

int mem_heap_free(void *addr , int count)
{
	//todo: implement
	return 0;
}

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


