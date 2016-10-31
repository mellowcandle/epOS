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
#define DEBUG

#include <mmodules.h>
#include <printk.h>
#include <types.h>
#include <mem/memory.h>
#include <bits.h>
#include <elf.h>
#include <lib/string.h>

typedef void (*call_module_t)(void);


static void mmodules_run(multiboot_module_t *module)
{
	FUNC_ENTER();
	uint32_t pages_count;
	call_module_t start_program;

	pr_info("Module: 0x%x - 0x%x params: %s\r\n", module->mod_start, module->mod_end, (char *) module->cmdline);
	pages_count = module->mod_end - module->mod_start;
	pages_count = divide_up(pages_count, PAGE_SIZE);

	mem_map_con_pages(PAGE_ALIGN_DOWN(module->mod_start), pages_count, 0, READ_WRITE_KERNEL);

	/* Run the module here */
	start_program = (call_module_t) 0;//module->mod_start;
	pr_debug("module start address: %x\r\n", (int) start_program);
	start_program();

	mem_page_unmap_multiple(0, pages_count);

	FUNC_LEAVE();
}

void mmodules_parse(multiboot_info_t *mbi)
{
	FUNC_ENTER();
	uint32_t i;
	multiboot_module_t *module;

	if (!(mbi->flags & MULTIBOOT_INFO_MODS))
	{
		return;
	}

	pr_info("Detected %u multiboot modules, located at: 0x%x \r\n", mbi->mods_count, mbi->mods_addr);

	// Identity map module description

	mem_identity_map(PAGE_ALIGN_DOWN(mbi->mods_addr), READ_WRITE_KERNEL);

	for (i = 0; i < mbi->mods_count; i++)
	{
		module = (multiboot_module_t *)(mbi->mods_addr + (i * sizeof(multiboot_module_t)));
		mmodules_run(module);
	}

	mem_page_unmap(mbi->mods_addr);
}
