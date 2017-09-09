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

#include <kernel.h>
#include <printk.h>
#include <types.h>
#include <elf.h>
#include <cpu.h>

extern elf_t kernel_elf;

void print_stack_trace();
void panic()
{
	disable_irq();
	printk("Kernel panic !\r\n");

	print_stack_trace();

	while (1)
		;
}
void print_stack_trace()
{
	uint32_t *ebp, *eip;
	int offset;
	const char *symbol;
	printk("Backtrace:\r\n");
	__asm volatile("mov %%ebp, %0" : "=r"(ebp));

	while (ebp)
	{
		eip = ebp + 1;
		symbol = elf_lookup_symbol(*eip, &kernel_elf, &offset);
		printk("--> [0x%x] %s", *eip, symbol);

		if (offset != -1)
		{
			printk("+%u", offset);
		}

		printk("\r\n");
		ebp = (uint32_t *) *ebp;
	}
}
