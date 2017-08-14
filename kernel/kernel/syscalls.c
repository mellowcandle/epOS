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
#include <scheduler.h>
#include <isr.h>
#include <printk.h>

#define SYSCALLS_COUNT 4

int syscall_open(char * file)
{
	FUNC_ENTER();
	return 0;
}

int syscall_close(int fd)
{
	FUNC_ENTER();
	return 0;
}

int syscall_read(int fd, char * buf, int len)
{
	FUNC_ENTER();
	return 0;
}

int syscall_write(int fd, char * buf, int len)
{
	FUNC_ENTER();
	return 0;
}

static void *syscalls[SYSCALLS_COUNT] =
{
   &syscall_open,
   &syscall_close,
   &syscall_read,
   &syscall_write
};


static void syscall_handler(registers_t regs)
{

  if (regs.eax >= SYSCALLS_COUNT)
	return;
	pr_info("syscall handler: %u\r\n", regs.eax);
	void *location = syscalls[regs.eax];
	// We don't know how many parameters the function wants, so we just
    // push them all onto the stack in the correct order. The function will
    // use all the parameters it wants, and we can pop them all back off afterwards.
    int ret;
    asm volatile (" \
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
    " : "=a" (ret) : "r" (regs.edi), "r" (regs.esi), "r" (regs.edx), "r" (regs.ecx), "r" (regs.ebx), "r" (location));
    regs.eax = ret;
}

void init_syscalls()
{
	register_interrupt_handler (0x40, &syscall_handler);
}

