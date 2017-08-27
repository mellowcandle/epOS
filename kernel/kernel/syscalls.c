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

#include <syscall.h>
#include <scheduler.h>
#include <isr.h>
#include <printk.h>
#include <lib/string.h>
#include <cpu.h>

#define SYSCALLS_COUNT 18

int syscall_exit(void)
{
	FUNC_ENTER();
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

	if (fd == 1)
	{
		printk("%s", buf);
	}

	return 0;
}



int syscall_close(int file)
{
	return -1;
}

int syscall_execve(char *name, char **argv, char **env)
{
//	errno = ENOMEM;
	return -1;
}

int syscall_fork(void)
{
//	errno = EAGAIN;
	return -1;
}

int syscall_fstat(int file, struct stat *st)
{
	FUNC_ENTER();
//	st->st_mode = S_IFCHR;
	return 0;
}

int syscall_getpid(void)
{
	FUNC_ENTER();
	return 1;
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
	FUNC_ENTER();
	return -1;
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


