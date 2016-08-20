#include "OS_types.h"
#include "boot/multiboot.h"
#include "video/VIDEO_textmode.h"
#include "kernel/isr.h"
#include "printk.h"
void kmain(void)
{
   extern uint32_t magic;
   extern void *mbd;
   int a;
   multiboot_info_t *mbi = mbd;

   if ( magic != MULTIBOOT_BOOTLOADER_MAGIC )
   {
      /* Something went not according to specs. Print an error */
      /* message and halt, but do *not* rely on the multiboot */
      /* data structure. */
           return;
   }
 
   /* You could either use multiboot.h */
   /* (http://www.gnu.org/software/grub/manual/multiboot/multiboot.html#multiboot_002eh) */
   /* or do your offsets yourself. The following is merely an example. */ 
   //char * boot_loader_name =(char*) ((long*)mbd)[16];
 
   /* Print a letter to screen to see everything is working: */
//   volatile unsigned char *videoram = (unsigned char *)0xB8000;
//   videoram[0] = 65; /* character 'A' */
//   videoram[1] = 0x07; /* light grey (7) on black (0). */

   VIDEO_clear_screen ();
   printk("EP-OS by Ramon Fried, all rights reservered.\r\n");

   printk("Initializing GDT......");
   Init_GDT();
   printk("Done\r\n");
   printk("Initializing IDT......");
   Init_IDT();
   printk("Done\r\n");
 
   disable_i8259();
   enableAPIC();

//   init_timer(50);
   //printk("Checking for APIC support......");


	printk("APIC was enabled succesfully\r\n");

	mem_init(mbi);

	while(1);
}
