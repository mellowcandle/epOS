/*
 * interrupts.c
 *
 *  Created on: Mar 30, 2013
 *      Author: gombotz
 */
#include <OS_types.h>
#include <printk.h>
#include <kernel/ports.h>
#include <kernel/isr.h>


isr_t interrupt_handlers[256];

void register_interrupt_handler(uint8_t n, isr_t handler)
{
    interrupt_handlers[n] = handler;
}

// This gets called from our ASM interrupt handler stub.
void isr_handler(registers_t regs)
{
    //VIDEO_print_string("recieved interrupt\n");
	printk("Recieved interrupt\r\n");
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
    	PORT_outb(0xA0, 0x20);
    }
    // Send reset signal to master. (As well as slave, if necessary).
    PORT_outb(0x20, 0x20);

    if (interrupt_handlers[regs.int_no] != 0)
    {
        isr_t handler = interrupt_handlers[regs.int_no];
        handler(regs);
    }

}
