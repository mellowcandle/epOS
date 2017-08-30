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
#include <process.h>
#include <tar.h>

typedef void (*call_module_t)(void);
extern int init_ramfs(addr_t phy_addr, uint32_t len);
extern int ramfs_find_node(const char * name, tar_header_t **ptr);
extern uint32_t ramfs_atoi(const char *in, uint32_t len);

static void mmodules_run(multiboot_module_t *module)
{
	FUNC_ENTER();
	uint32_t pages_count;
	tar_header_t * tar_header;
	void * elf_ptr;
	uint32_t file_size;

	pr_info("Module: 0x%x - 0x%x \r\n", module->mod_start, module->mod_end);//, (char *) module->cmdline);
//	pages_count = module->mod_end - module->mod_start;
//	pages_count = divide_up(pages_count, PAGE_SIZE);
	init_ramfs(module->mod_start, module->mod_end - module->mod_start);
	if (ramfs_find_node("./bin/test1", &tar_header))
	{
		pr_fatal("Can't find init process...\r\n");
		while(1);
	}
	elf_ptr = &tar_header->charptr[512]; //On the edge of the tar header lays the file contents
	file_size = ramfs_atoi(tar_header->header.size, 11);

	prepare_init_task(elf_ptr, file_size);
	pr_info("Spawning init process\r\n");
	while (1)
		;
	FUNC_LEAVE();
}

void mmodules_parse(multiboot_info_t *mbi)
{
	FUNC_ENTER();
	uint32_t i;
	multiboot_module_t modules[mbi->mods_count];

	if (!(mbi->flags & MULTIBOOT_INFO_MODS))
	{
		return;
	}

	pr_info("Detected %u multiboot modules, located at: 0x%x \r\n", mbi->mods_count, mbi->mods_addr);

	// Identity map module description

	mem_identity_map(PAGE_ALIGN_DOWN(mbi->mods_addr), READ_WRITE_KERNEL);
	memcpy(modules, (void *) mbi->mods_addr, mbi->mods_count * sizeof(multiboot_module_t));
	mem_page_unmap((void *)mbi->mods_addr);

	for (i = 0; i < mbi->mods_count; i++)
	{
		pr_info("Running module: %u\r\n", i);
		mmodules_run(&modules[i]);
	}

}
