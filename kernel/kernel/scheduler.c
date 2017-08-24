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

#include <process.h>
#include <lib/list.h>
#include <scheduler.h>
#include <printk.h>
#include <apic.h>
#include <cpu.h>

static LIST(running_tasks);
static LIST(stopped_tasks);

extern uint32_t *current_pdt;

static task_t *current_task = NULL;
static task_t idle_task = {
	.pid = 0,
	.parent_pid = 0,
	.state = TASK_RUNNING,
	.regs.eflags = 0x202,
#define SEGSEL_KERNEL_CS 0x08
#define SEGSEL_KERNEL_DS 0x10
	.regs.ss = SEGSEL_KERNEL_DS,
	.regs.cs = SEGSEL_KERNEL_CS,
};


static task_t *_scheduler_get_next_running_task()
{

	if (list_is_empty(&running_tasks)) {
		return &idle_task;
	}
	else {
		return list_entry(list_remove_entry(running_tasks.next), task_t, list);
	}

}

task_t * get_current_task()
{
	return current_task;
}

void scheduler_switch_task(registers_t *regs)
{
	FUNC_ENTER();
	if (current_task == NULL)
	{
		return;    // scheduler wasn't loaded yet
	}

	disable_irq(); // Interrupts will be restored on IRET
	save_registers(current_task, regs);
	list_add_tail(&current_task->list, &running_tasks);

	current_task = _scheduler_get_next_running_task();
	pr_debug("Switching to task %u...\r\n", current_task->pid);
	apic_eoi();

	switch_to_task(current_task);

}
void scheduler_add_task(task_t *task)
{
	task->state = TASK_RUNNING;
	list_add_tail(&task->list, &running_tasks);
}

void scheduler_remove_task(task_t *task)
{
	task->state = TASK_STOPPED;
	list_remove_entry(&task->list);
}

void  idle_function()
{
	while(1) {
		cpu_relax();
	}
}
static void prepare_idle_task()
{
	/* Setup the idle task */
	idle_task.pdt_virt_addr = current_pdt;
	// Allocate kernel stack
	idle_task.kernel_stack_phy_addr = mem_get_page();
	if (!idle_task.kernel_stack_phy_addr)
	{
		pr_fatal("Can't get free page\r\n");
		panic();
	}
	idle_task.kernel_stack_virt_addr = mem_page_map_kernel(idle_task.kernel_stack_phy_addr, 1, READ_WRITE_KERNEL);
	idle_task.kernel_stack_pointer = idle_task.kernel_stack_virt_addr + PAGE_SIZE - 4;
	idle_task.regs.esp = (addr_t) idle_task.kernel_stack_pointer;
	idle_task.regs.eip = (addr_t) &idle_function;
	idle_task.regs.eip = (addr_t) &idle_function;
}

void scheduler_start()
{
	disable_irq(); // Interrupts will be restored in user space

	prepare_idle_task();

	current_task = _scheduler_get_next_running_task();
	switch_to_task(current_task);
	/* Should not get here */
	while (1);
}
void save_registers(task_t * current_task, registers_t * regs) {

/* Save registers */
	current_task->regs.eax = regs->eax;
	current_task->regs.ebx = regs->ebx;
	current_task->regs.ecx = regs->ecx;
	current_task->regs.edx = regs->edx;
	current_task->regs.ebp = regs->ebp;
	current_task->regs.esi = regs->esi;
	current_task->regs.edi = regs->edi;
	current_task->regs.cs = regs->cs;
	current_task->regs.ss = regs->ss;
	current_task->regs.eip = regs->eip;
	current_task->regs.eflags = regs->eflags;
	if(current_task->type == TASK_USER) {
		current_task->regs.esp = regs->useresp;
	}
	else {
		current_task->regs.esp = regs->esp + 20;
		current_task->regs.ss = SEGSEL_KERNEL_DS;
	}

}
