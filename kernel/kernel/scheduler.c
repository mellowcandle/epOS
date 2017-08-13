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

#include <process.h>
#include <lib/list.h>
#include <scheduler.h>
#include <printk.h>
#include <apic.h>
#include <cpu.h>

static LIST(running_tasks);
static LIST(stopped_tasks);

static task_t *current_task = NULL;

static task_t *_scheduler_get_next_running_task()
{
	//TODO: Implement the scheduler here.
	return list_first_entry(&running_tasks, task_t, list);
}

task_t * get_current_task()
{
	return current_task;
}

void scheduler_switch_task(registers_t regs)
{
	if (current_task == NULL)
	{
		return;    // scheduler wasn't loaded yet
	}
	disable_irq(); // Interrupts will be restored in user space

	pr_info("Switching to task %u...\r\n", current_task->pid);

	/* Save registers */
	current_task->regs.eax = regs.eax;
	current_task->regs.ebx = regs.ebx;
	current_task->regs.ecx = regs.ecx;
	current_task->regs.edx = regs.edx;
	current_task->regs.ebp = regs.ebp;
	current_task->regs.esp = regs.esp + 12; // to exclude what was pushed in IRQ
	current_task->regs.esi = regs.esi;
	current_task->regs.edi = regs.edi;
	current_task->regs.cs = regs.cs;
	current_task->regs.ss = regs.ss;
	current_task->regs.eip = regs.eip;
	current_task->regs.eflags = regs.eflags;

	current_task = _scheduler_get_next_running_task();
//	apic_eoi();

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

void scheduler_start()
{
	disable_irq(); // Interrupts will be restored in user space
	current_task = _scheduler_get_next_running_task();
	switch_to_task(current_task);
	/* Should not get here */
	while (1);
}

