#include <kernel.h>
#include <printk.h>

void panic() 
{
	printk("Kernel panic !\r\n");
	while (1)
		;
}
