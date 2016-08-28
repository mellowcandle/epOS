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

	if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
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

	while (1);
}
