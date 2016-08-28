#include <OS_types.h>
#include <boot/multiboot.h>
#include <video/VIDEO_textmode.h>
#include <kernel/isr.h>
#include <printk.h>
#include <mem/mem_pages.h>
#include <kernel/cpu.h>
#include <serial.h>
void kmain(void)
{
   extern uint32_t magic;
   extern void *mbd;
   multiboot_info_t *mbi = mbd;

   if ( magic != MULTIBOOT_BOOTLOADER_MAGIC )
   {
	  /* Something went not according to specs. Print an error */
	  /* message and halt, but do *not* rely on the multiboot */
	  /* data structure. */
		   return;
   }
	init_serial();


	//   VIDEO_clear_screen();
   printk("EP-OS by Ramon Fried, all rights reservered.\r\n");

	printk("Initializing GDT......\r\n");
	Init_GDT();
	printk("Initializing IDT......\r\n");
	Init_IDT();

	disable_i8259();
//	enableAPIC();

//   init_timer(50);
   //printk("Checking for APIC support......");


	printk("APIC was enabled succesfully\r\n");
	mem_init(mbi);

	/* addr_t addr;                              */
	/* addr = mem_page_get();                    */
	/* printk("Got a mem page: 0x%x\r\n", addr); */
	/* mem_page_free(addr);                      */

	while(1);
}
