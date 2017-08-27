
#include <kernel.h>
#include <types.h>
#include <printk.h>
#include <cpu.h>
#include <isr.h>

static void gen_protection_fault(registers_t *regs)
{
	pr_fatal("General Protection fault: 0x%x\r\n", regs->err_code);

	while (1)
		;
}
static void double_fault(registers_t *regs)
{
	pr_fatal("Double fault\r\n");
	panic();
}

void cpu_init()
{
	gdt_init();
	idt_init();

	register_interrupt_handler(8, &double_fault);
	register_interrupt_handler(13, &gen_protection_fault);
}

