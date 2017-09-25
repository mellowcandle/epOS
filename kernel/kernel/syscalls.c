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

#include <syscall.h>
#include <errno.h>
#include <stat.h>
#include <kmalloc.h>
#include <scheduler.h>
#include <isr.h>
#include <printk.h>
#include <lib/string.h>
#include <cpu.h>
#include <process.h>


#define SYSCALLS_COUNT 18

int syscall_exit(int ret)
{
	pr_debug("+syscall_exit: %d\r\n", ret);
	task_t * task = get_current_task();
	task->exit_value = ret;

	process_cleanup(task);
	schedule();
	/* Should not return from this */

	return 0;
}


int syscall_open(char *file, int flags, int mode)
{
	FUNC_ENTER();

	if (!strcmp(file, "/dev/console"))
	{
		return 1;
	}
	else

	{
		return 0;
	}
}

int syscall_read(int fd, char *buf, int len)
{
	FUNC_ENTER();

	return 0;
}

int syscall_write(int fd, char *buf, int len)
{
	FUNC_ENTER();

	switch(fd)
	{
	case 1:
	case 2:
		printk("%s", buf);
		break;
	default:
		break;
	}

	return len;
}



int syscall_close(int file)
{
	pr_debug("+syscall_close: file: %d\r\n", file);
	return 0;
}

int syscall_execve(char *name, char **argv, char **env)
{
//	errno = ENOMEM;
	return -1;
}

int syscall_fork(void)
{
	task_t * task = clone(get_current_task());
	if (task) {
		return task->pid;
	}
	return -ENOMEM;
}

int syscall_fstat(int file, struct stat *st)
{
	FUNC_ENTER();
	switch (file)
	{
	case 0:
	case 1:
	case 2:
		st->st_mode = S_IFCHR;
		break;
	default:
		break;
	}

	return 0;
}

int syscall_getpid(void)
{
	FUNC_ENTER();
	task_t * task = get_current_task();
	return task->pid;
}

int syscall_isatty(int file)
{
	FUNC_ENTER();
	return 1;
}

int syscall_kill(int pid, int sig)
{
	FUNC_ENTER();
//  errno = EINVAL;
	return -1;
}

int syscall_link(char *old, char *new)
{
	FUNC_ENTER();
//	errno = EMLINK;
	return -1;
}

int syscall_lseek(int file, int ptr, int dir)
{
	FUNC_ENTER();
	return 0;
}

int syscall_sbrk(int incr)
{
	pr_debug("+syscall_sbrk: increment with: %d bytes\r\n", incr);

	int ret;
	task_t * task = get_current_task();
	memblock_t *block = kmalloc(sizeof(memblock_t));
	if (!block) {
		pr_error("Memory allocation failure\r\n");
		return -ENOMEM;
	}

	block->count = divide_up(incr, PAGE_SIZE);
	block->v_addr = task->heap_top;
	block->p_addr = mem_get_pages(block->count);
	if (!block->p_addr) {
		pr_warn("Out of physical memory\r\n");
		goto cleanup2;
	}

	void *tmp = mem_page_map_kernel(block->p_addr, block->count, READ_WRITE_KERNEL | PTE_TEMPORARY);
	if (!tmp) {
		pr_error("kernel mapping error\r\n");
		goto cleanup1;
	}
	memset(tmp,  0, block->count * PAGE_SIZE);
	mem_page_unmap_multiple(tmp, block->count);

	ret = mem_pages_map_pdt_multiple(task->pdt_virt_addr, block->p_addr, block->v_addr, block->count, READ_WRITE_USER);
	if (ret)
	{
		pr_error("Can't sbrk\r\n");
		goto cleanup1;
	}
	task->heap_top += block->count * PAGE_SIZE;
	list_add(&block->list, &task->mapped_memory_list);
	return (int) block->v_addr;

cleanup1:
	mem_free_pages(block->p_addr, block->count);
cleanup2:
	kfree(block);
	return -ENOMEM;
}

struct stat;
int syscall_stat(char *file, struct stat *st)
{
	FUNC_ENTER();
//	st->st_mode = S_IFCHR;
	return 0;
}

struct tms;
int syscall_times(struct tms *buf)
{
	FUNC_ENTER();
	return -1;
}

int syscall_unlink(char *name)
{
	FUNC_ENTER();
//	errno = ENOENT;
	return -1;
}

int syscall_wait(int *status)
{
	FUNC_ENTER();
//	errno = ECHILD;
	return -1;
}

static void *syscalls[SYSCALLS_COUNT] =
{
	&syscall_exit,
	&syscall_close,
	&syscall_execve,
	&syscall_fork,
	&syscall_fstat,
	&syscall_getpid,
	&syscall_isatty,
	&syscall_kill,
	&syscall_link,
	&syscall_lseek,
	&syscall_open,
	&syscall_read,
	&syscall_sbrk,
	&syscall_stat,
	&syscall_times,
	&syscall_unlink,
	&syscall_wait,
	&syscall_write
};

static void syscall_handler(registers_t *regs)
{
	disable_irq(); // Interrupts will be restored in user space
	task_t *current = get_current_task();

	pr_debug("syscall handler: %u\r\n", regs->eax);

	if (regs->eax >= SYSCALLS_COUNT)
	{
		return;
	}

	save_registers(current, regs);

	void *location = syscalls[regs->eax];
	// We don't know how many parameters the function wants, so we just
	// push them all onto the stack in the correct order. The function will
	// use all the parameters it wants, and we can pop them all back off afterwards.
	int ret;
	asm volatile(" \
			push %1; \
			push %2; \
			push %3; \
			push %4; \
			push %5; \
			call *%6; \
			pop %%ebx; \
			pop %%ebx; \
			pop %%ebx; \
			pop %%ebx; \
			pop %%ebx; \
			" : "=a"(ret) : "r"(regs->edi), "r"(regs->esi), "r"(regs->edx), "r"(regs->ecx), "r"(regs->ebx), "r"(location));
	current->regs.eax = ret;

	switch_to_task(current);

}
void init_syscalls()
{
	register_interrupt_handler(0x40, &syscall_handler);
}


