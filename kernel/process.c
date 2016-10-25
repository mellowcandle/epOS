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

#include <types.h>
#include <mem/memory.h>
#include <printk.h>
#include <kmalloc.h>
#include <process.h>

static uint32_t pid_counter = 0;

uint32_t get_next_pid()
{
	return ++pid_counter;
}


task_t *clone(task_t *parent)
{
	task_t *new = kmalloc(sizeof(task_t));

	if (!new)
	{
		pr_error("No memory to create process\r\n");
		return NULL;
	}

	new->parent_pid = parent->pid;
	new->regs = parent->regs;
	new->pdt_virt_addr = mem_calloc_pdt(&new->pdt_phy_addr);

	if (!new->pdt_virt_addr)
	{
		pr_error("Can't create page directory\r\n");
		goto fail;
	}

	return new;
fail:
	kfree(new);
	return NULL;
}
