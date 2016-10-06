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
//#define DEBUG

#include <mmodules.h>
#include <printk.h>
#include <types.h>
#include <mem/memory.h>
#include <kernel/bits.h>
#include <elf.h>
#include <lib/string.h>

typedef void (*call_module_t)(void);
int elf_from_multiboot(multiboot_elf_section_header_table_t *elf_sec, elf_t *elf);

elf_t kernel_elf;

static void mmodules_run(multiboot_module_t *module)
{
	FUNC_ENTER();
	uint32_t pages_count;
	call_module_t start_program;

	pr_info("Module: 0x%x - 0x%x params: %s\r\n", module->mod_start, module->mod_end, (char *) module->cmdline);
	pages_count = module->mod_end - module->mod_start;
	pages_count = divide_up(pages_count, PAGE_SIZE);

	mem_identity_map_multiple(module->mod_start, 0, pages_count);

	/* Run the module here */
	start_program = (call_module_t) module->mod_start;
	pr_debug("moduel start address: %x\r\n", (int) start_program);
	start_program();

	mem_page_unmap_multiple(module->mod_start, pages_count);

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

	mem_identity_map(mbi->mods_addr, 0);

	for (i = 0; i < mbi->mods_count; i++)
	{
		module = (multiboot_module_t *)(mbi->mods_addr + (i * sizeof(multiboot_module_t)));
		mmodules_run(module);
	}

	mem_page_unmap(mbi->mods_addr);
}


void ksymbol_init(multiboot_info_t *mbi)
{

	FUNC_ENTER();

	multiboot_elf_section_header_table_t *elf_sec;

	if (!(mbi->flags & MULTIBOOT_INFO_ELF_SHDR))
	{
		return;
	}

	elf_sec = &mbi->u.elf_sec;

	pr_debug("multiboot header: sections %u size %u addr: 0x%x shndx %u\r\n",
	         elf_sec->num, elf_sec->size, elf_sec->addr, elf_sec->shndx);

	mem_identity_map_multiple(PAGE_ALIGN_DOWN(elf_sec->addr), 0, divide_up(elf_sec->size, PAGE_SIZE));
	elf_from_multiboot(elf_sec, &kernel_elf);
}


int elf_from_multiboot(multiboot_elf_section_header_table_t *elf_sec, elf_t *elf)
{
	FUNC_ENTER();
	unsigned int i;
	elf_section_header_t *sh = (elf_section_header_t *)elf_sec->addr;
	uint32_t shstrtab = sh[elf_sec->shndx].addr;

	mem_identity_map(PAGE_ALIGN_DOWN(shstrtab), 0);

	for (i = 0; i < elf_sec->num; i++)
	{
		const char *name = (const char *)(shstrtab + sh[i].name);

		if (!strcmp(name, ".strtab"))
		{
			elf->strtab = (const char *)sh[i].addr;
			elf->strtabsz = sh[i].size;
			pr_debug("Identity maping: 0x%x pages: %u\r\n", (addr_t) elf->strtab, divide_up(elf->strtabsz, PAGE_SIZE));
			mem_identity_map_multiple(PAGE_ALIGN_DOWN((addr_t)elf->strtab), 0, divide_up(elf->strtabsz, PAGE_SIZE));
		}

		if (!strcmp(name, ".symtab"))
		{
			elf->symtab = (elf_symbol_t *)sh[i].addr;
			elf->symtabsz = sh[i].size;
			pr_debug("Identity maping: 0x%x pages: %u\r\n", (addr_t) elf->symtab, divide_up(elf->symtabsz, PAGE_SIZE));
			mem_identity_map_multiple(PAGE_ALIGN_DOWN((addr_t)elf->symtab), 0, divide_up(elf->symtabsz, PAGE_SIZE));
		}
	}

	return 0;
}

const char *elf_lookup_symbol(uint32_t addr, elf_t *elf, int *offset)
{
	FUNC_ENTER();
	unsigned int i;

	for (i = 0; i < (elf->symtabsz / sizeof(elf_symbol_t)); i++)
	{
		if (ELF32_ST_TYPE(elf->symtab[i].info) != 0x2)
		{
			continue;
		}

		if ((addr >= elf->symtab[i].value) &&
		        (addr < (elf->symtab[i].value + elf->symtab[i].size)))
		{
			const char *name = (const char *)((uint32_t)elf->strtab + elf->symtab[i].name);
			*offset = addr - elf->symtab[i].value;
			return name;
		}
	}

	*offset = -1; //invalid
	return "??????";
}
