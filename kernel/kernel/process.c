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
#define DEBUG

#include <types.h>
#include <mem/memory.h>
#include <printk.h>
#include <kmalloc.h>
#include <process.h>
#include <scheduler.h>
#include <tss.h>

static uint32_t pid_counter = 0;
extern uint32_t *current_pdt;

/* segements */
#define SEGSEL_KERNEL_CS 0x08
#define SEGSEL_KERNEL_DS 0x10
#define SEGSEL_USER_SPACE_CS 0x18
#define SEGSEL_USER_SPACE_DS 0x20


void mem_switch_page_directory(addr_t new_dir);

/* defined in scheduler.s */
void run_user_task(task_registers_t *registers);
void run_kernel_task(task_registers_t *registers);

uint32_t get_next_pid()
{
	return ++pid_counter;
}

void prepare_init_task(void *physical, uint32_t count)
{
	FUNC_ENTER();
	dump_pdt_indirect(current_pdt);
	task_t *new = kzalloc(sizeof(task_t));

	if (!new)
	{
		pr_error("No memory to create process\r\n");
		return;
	}

	pr_info("%d\r\n", __LINE__);
	new->pid = get_next_pid();
	new->parent_pid = new->pid; // init process

	// Allocate kernel stack
	new->kernel_stack_phy_addr = mem_get_page();

	if (!new->kernel_stack_phy_addr)
	{
		pr_error("Can't get free page\r\n");
		goto fail1;
	}

	pr_info("%d\r\n", __LINE__);

	new->kernel_stack_virt_addr = mem_page_map_kernel(new->kernel_stack_phy_addr, 1, READ_WRITE_KERNEL);
	new->kernel_stack_pointer = new->kernel_stack_virt_addr + PAGE_SIZE - 4;
	pr_debug("New process stack pointer: 0x%x, stack frame: 0x%x\r\n", (uint32_t) new->kernel_stack_pointer, (uint32_t) new->kernel_stack_virt_addr);
	new->pdt_virt_addr = mem_calloc_pdt(&new->pdt_phy_addr);
	pr_info("%d\r\n", __LINE__);

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

	mem_pages_map_pdt_multiple(new->pdt_virt_addr, (addr_t) physical, 0, count, READ_WRITE_USER);
	pr_info("%d\r\n", __LINE__);

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
	pr_info("%d\r\n", __LINE__);

	new->regs.eflags = 0x202; //TODO: understand why
	new->regs.ss = SEGSEL_KERNEL_DS;// | 0x03;
	new->regs.cs = SEGSEL_KERNEL_CS;// | 0x03;
	new->regs.esp = (uint32_t) new->kernel_stack_pointer;
	/* new->regs.esp = (uint32_t) new->stack_virt_addr - 4; */
	new->regs.eip = 0;
	dump_pdt_indirect(current_pdt);
	pr_info("%d\r\n", __LINE__);
	dump_pdt_indirect(new->pdt_virt_addr);
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

void switch_to_task(task_t *task)
{
	tss_set_kernel_stack(0x10, (uint32_t)task->kernel_stack_virt_addr);
	mem_switch_page_directory(task->pdt_phy_addr);
	run_kernel_task(&task->regs);
}
