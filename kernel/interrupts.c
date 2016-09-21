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
#include <printk.h>
#include <cpu.h>
#include <isr.h>


isr_t interrupt_handlers[256];

void register_interrupt_handler(uint8_t n, isr_t handler)
{
	interrupt_handlers[n] = handler;
}

// This gets called from our ASM interrupt handler stub.
void isr_handler(registers_t regs)
{
	//VIDEO_print_string("recieved interrupt\n");
	printk("Recieved interrupt number: %u\r\n", regs.int_no);

	if (interrupt_handlers[regs.int_no] != 0)
	{
		isr_t handler = interrupt_handlers[regs.int_no];
		handler(regs);
	}
}

// This gets called from our ASM interrupt handler stub.
void irq_handler(registers_t regs)
{
	// Send an EOI (end of interrupt) signal to the PICs.
	// If this interrupt involved the slave.
	if (regs.int_no >= 40)
	{
		// Send reset signal to slave.
		outb(0xA0, 0x20);
	}

	// Send reset signal to master. (As well as slave, if necessary).
	outb(0x20, 0x20);

	if (interrupt_handlers[regs.int_no] != 0)
	{
		isr_t handler = interrupt_handlers[regs.int_no];
		handler(regs);
	}

}

void irq_reg_dump(registers_t *regs)
{
	printk("\r\n**************************************************\r\n");
	printk("EIP: 0x%x CS: 0x%x EFLAGS: 0x%x\r\n",
	       regs->eip, regs->cs, regs->eflags);
	printk("DS: 0x%x EDI: 0x%x ESI: 0x%x EBP: 0x%x EBX: 0x%x\r\n",
	       regs->ds, regs->edi, regs->esi, regs->ebp, regs->ebx);
	printk("EDX: 0x%x ECX: 0x%x EAX: 0x%x USERESP: 0x%x SS: 0x%x\r\n",
	       regs->edx, regs->ecx, regs->eax, regs->useresp, regs->ss);
	printk("\r\n**************************************************\r\n");
}

