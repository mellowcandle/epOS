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
//#define DEBUG

#include <types.h>
#include <mem/memory.h>
#include <printk.h>
#include <kmalloc.h>
#include <process.h>
#include <scheduler.h>
#include <tss.h>
#include <elf.h>
#include <lib/string.h>

static uint32_t pid_counter = 0;
extern uint32_t *current_pdt;

/* segements */
#define SEGSEL_KERNEL_CS 0x08
#define SEGSEL_KERNEL_DS 0x10
#define SEGSEL_USER_SPACE_CS 0x18
#define SEGSEL_USER_SPACE_DS 0x20

void mem_switch_page_directory(addr_t new_dir);

uint32_t get_next_pid()
{
	return ++pid_counter;
}

void prepare_init_task(void *addr, uint32_t count)
{
	FUNC_ENTER();
	task_t *new = kzalloc(sizeof(task_t));

	if (!new)
	{
		pr_error("No memory to create process\r\n");
		return;
	}

	LIST_HEAD_INIT(&new->mapped_memory_list);

	new->pid = get_next_pid();
	new->parent_pid = new->pid; // init process
	new->heap_top = 0;
	new->type = TASK_USER;
	// Allocate kernel stack
	new->kernel_stack_phy_addr = mem_get_page();

	if (!new->kernel_stack_phy_addr)
	{
		pr_error("Can't get free page\r\n");
		goto fail1;
	}

	new->kernel_stack_virt_addr = mem_page_map_kernel(new->kernel_stack_phy_addr, 1, READ_WRITE_KERNEL);
	new->kernel_stack_pointer = new->kernel_stack_virt_addr + PAGE_SIZE - 4;
	pr_debug("New process stack pointer: 0x%x, stack frame: 0x%x\r\n", (uint32_t) new->kernel_stack_pointer, (uint32_t) new->kernel_stack_virt_addr);
	new->pdt_virt_addr = mem_calloc_pdt(&new->pdt_phy_addr);

	if (!new->pdt_virt_addr)
	{
		pr_error("Can't create page directory\r\n");
		goto fail2;
	}

	if (clone_pdt(current_pdt, new->pdt_virt_addr, new->pdt_phy_addr))
	{
		pr_error("Failed cloning process\r\n");
		goto fail3;
	}


	/* Parse the ELF and copy the sections to memory */

	if (load_elf(new, addr))
	{
		pr_fatal("Can't load ELF to memory\r\n");
		panic();
	}

	//
	//mem_pages_map_pdt_multiple(new->pdt_virt_addr, (addr_t) physical, 0, count, READ_WRITE_USER);

	// Allocate user stack
	new->stack_virt_addr = (void *) KERNEL_VIRTUAL_BASE - PAGE_SIZE;
	new->stack_phy_addr	 = mem_get_page();

	if (!new->stack_phy_addr)
	{
		pr_error("Can't get free page\r\n");
		goto fail4;
	}

	// Map the user stack
	mem_page_map_pdt(new->pdt_virt_addr, new->stack_phy_addr, new->stack_virt_addr, READ_WRITE_USER);

	/* Temporarily map the user stack in order to push argc, argv and env */
	uint32_t *stack = mem_page_map_kernel_single(new->stack_phy_addr, READ_WRITE_KERNEL | PTE_TEMPORARY);
	memset(stack,0xff,PAGE_SIZE);
	stack[1023] = 1; //argc
	stack[1022] = 0; //argv
	stack[1021] = 0; //env
	mem_page_unmap(stack);

	new->regs.eflags = 0x202;
	new->regs.ss = SEGSEL_USER_SPACE_DS | 0x03;
	new->regs.cs = SEGSEL_USER_SPACE_CS | 0x03;
	new->regs.esp = KERNEL_VIRTUAL_BASE - 16; //argc + argv + env
	// Push argc and argv to the stack
	scheduler_add_task(new);

	FUNC_LEAVE();
	return;

fail4:
	mem_free_page(new->stack_phy_addr);
fail3:
	mem_release_pdt(new->pdt_phy_addr, new->pdt_virt_addr);
fail2:
	mem_page_unmap(new->kernel_stack_virt_addr);
	mem_free_page(new->kernel_stack_phy_addr);
fail1:
	kfree(new);

	FUNC_LEAVE();
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
	new->pid = get_next_pid();
	new->regs = parent->regs;
	new->pdt_virt_addr = mem_calloc_pdt(&new->pdt_phy_addr);

	if (!new->pdt_virt_addr)
	{
		pr_error("Can't create page directory\r\n");
		goto fail;
	}

	if (clone_pdt(current_pdt, new->pdt_virt_addr, new->pdt_phy_addr))
	{
		pr_error("Failed cloning process\r\n");
	}

	mem_switch_page_directory(new->pdt_phy_addr);
	return new;
fail:
	kfree(new);
	return NULL;
}
void dump_task_state(task_t *task)
{
	pr_debug("Task dump\r\n");
	pr_debug("eax: %x ebx: %x ecx: %x edx: %x ebp: %x esi: %x edi: %x ss:%x esp: %x eflags: %x cs: %x eip: %x \r\n",
	         task->regs.eax,
	         task->regs.ebx,
	         task->regs.ecx,
	         task->regs.edx,
	         task->regs.ebp,
	         task->regs.esi,
	         task->regs.edi,
	         task->regs.ss,
	         task->regs.esp,
	         task->regs.eflags,
	         task->regs.cs,
	         task->regs.eip);

}
void switch_to_task(task_t *task)
{
#ifdef DEBUG
	dump_task_state(task);
#endif
	tss_set_kernel_stack(0x10, (uint32_t)task->kernel_stack_pointer);

	if (task->type == TASK_USER)
	{
		mem_switch_page_directory(task->pdt_phy_addr);
		run_user_task(&task->regs);
	}
	else
	{
		run_kernel_task(&task->regs);
	}

}

int process_cleanup(task_t * task)
{
	memblock_t *block;

	/* TODO: Free the file descriptors */
	/* TODO: Notify parent via signal */

	/* Free the user stack */
	mem_free_page(task->stack_phy_addr);

	/* Free all process memory */
	list_for_each_entry(block, &task->mapped_memory_list, list)
	{
		pr_debug("Freeing block: V0x%x P0x%x, size: %u\r\n",
				block->v_addr, block->p_addr, block->count);
		mem_free_pages(block->p_addr, block->count);
		/* TODO: kfree the structures */

	}

	/* Free the kernel stack */
	mem_page_unmap(task->kernel_stack_virt_addr);
	mem_free_page(task->kernel_stack_phy_addr);


	/* TODO: add process to zombie list */
	return 0;
}
