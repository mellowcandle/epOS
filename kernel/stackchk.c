#include <kernel.h>
#include <printk.h>

#define STACK_CHK_GUARD 0xe2dee578

uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

__attribute__((noreturn))
void __stack_chk_fail(void)
{
	pr_fatal("Stack smashing detected\r\n");
	panic();
}
