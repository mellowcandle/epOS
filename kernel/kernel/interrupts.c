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
#include <printk.h>
#include <cpu.h>
#include <isr.h>
#include <apic.h>
#include <scheduler.h>
#include <lib/string.h>

isr_t interrupt_handlers[256];

void register_interrupt_handler(uint8_t n, isr_t handler)
{
	interrupt_handlers[n] = handler;
}

// This gets called from our ASM interrupt handler stub.
void isr_handler(registers_t regs)
{
	pr_warn("Recieved interrupt number: %u\r\n", regs.int_no);

	if (interrupt_handlers[regs.int_no] != 0)
	{
		isr_t handler = interrupt_handlers[regs.int_no];
		handler(&regs);
	}

}

// This gets called from our ASM interrupt handler stub.
void irq_handler(registers_t regs)
{
	if (interrupt_handlers[regs.int_no] != 0)
	{
		isr_t handler = interrupt_handlers[regs.int_no];
		handler(&regs);
	}

	apic_eoi();
	pr_debug("Recieved IRQ number: %u\r\n", regs.int_no);
}

void irq_reg_dump(registers_t *regs)
{
	printk("\r\n**************************************************************************************\r\n");
	printk("EIP: 0x%.8x CS:  0x%.8x EFLAGS: 0x%.8x\r\n",
	       regs->eip, regs->cs, regs->eflags);
	printk("DS:  0x%.8x EDI: 0x%.8x ESI:    0x%.8x EBP:     0x%.8x EBX: 0x%.8x\r\n",
	       regs->ds, regs->edi, regs->esi, regs->ebp, regs->ebx);
	printk("EDX: 0x%.8x ECX: 0x%.8x EAX:    0x%.8x USERESP: 0x%.8x SS:  0x%x\r\n",
	       regs->edx, regs->ecx, regs->eax, regs->useresp, regs->ss);
	printk("\r\n**************************************************************************************\r\n");
}

