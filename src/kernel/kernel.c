#include "OS_types.h"
#include "boot/multiboot.h"
#include "video/VIDEO_textmode.h"
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
 
   /* You could either use multiboot.h */
   /* (http://www.gnu.org/software/grub/manual/multiboot/multiboot.html#multiboot_002eh) */
   /* or do your offsets yourself. The following is merely an example. */ 
   //char * boot_loader_name =(char*) ((long*)mbd)[16];
 
   /* Print a letter to screen to see everything is working: */
//   volatile unsigned char *videoram = (unsigned char *)0xB8000;
//   videoram[0] = 65; /* character 'A' */
//   videoram[1] = 0x07; /* light grey (7) on black (0). */

   VIDEO_clear_screen ();
   VIDEO_print_string("EP-OS by Ramon Fried, all rights reservered.\r\n");

   VIDEO_print_string("Initializing GDT......");
   Init_GDT();
   VIDEO_print_string("Done\r\n");
   VIDEO_print_string("Initializing IDT......");
   Init_IDT();
   VIDEO_print_string("Done\r\n");

   //VIDEO_print_string("Checking for APIC support......");



   while(1);
}
