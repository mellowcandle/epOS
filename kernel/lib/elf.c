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

#include <elf.h>
#include <boot/multiboot.h>
#include <mem/memory.h>
#include <printk.h>
#include <lib/string.h>
#include <bits.h>

elf_t kernel_elf;

static int elf_from_multiboot(multiboot_elf_section_header_table_t *elf_sec, elf_t *elf)
{
	FUNC_ENTER();
	unsigned int i;
	uint32_t tmp_map;

	elf32_shdr *sh = (elf32_shdr*)elf_sec->addr;
	uint32_t shstrtab = sh[elf_sec->shndx].sh_addr;

	tmp_map = (addr_t) mem_page_map_kernel_single(PAGE_ALIGN_DOWN(shstrtab), READ_WRITE_KERNEL);
	shstrtab = tmp_map | (shstrtab & ~PAGE_MASK);

	pr_debug("shstrtab mapping: 0x%x -> 0x%x\r\n", sh[elf_sec->shndx].addr, shstrtab);

	for (i = 0; i < elf_sec->num; i++)
	{
		const char *name = (const char *)(shstrtab + sh[i].sh_name);

		if (!strcmp(name, ".strtab"))
		{
			elf->strtabsz = sh[i].sh_size;
			tmp_map = (addr_t) mem_page_map_kernel(PAGE_ALIGN_DOWN(sh[i].sh_addr), divide_up(elf->strtabsz, PAGE_SIZE), READ_WRITE_KERNEL);
			elf->strtab = (char *)(tmp_map | (sh[i].sh_addr & ~PAGE_MASK));
			pr_debug("strtab maping: 0x%x -> 0x%x pages: %u\r\n", sh[i].addr, (addr_t) elf->strtab, divide_up(elf->strtabsz, PAGE_SIZE));
		}

		if (!strcmp(name, ".symtab"))
		{
			elf->symtabsz = sh[i].sh_size;
			tmp_map = (addr_t) mem_page_map_kernel(PAGE_ALIGN_DOWN(sh[i].sh_addr), divide_up(elf->symtabsz, PAGE_SIZE), READ_WRITE_KERNEL);
			elf->symtab = (elf32_sym *)(tmp_map | (sh[i].sh_addr & ~PAGE_MASK));
			pr_debug("symtab maping: 0x%x -> 0x%x pages: %u\r\n", sh[i].addr, (addr_t) elf->symtab, divide_up(elf->symtabsz, PAGE_SIZE));

		}
	}

	return 0;
}

const char *elf_lookup_symbol(uint32_t addr, elf_t *elf, int *offset)
{
	FUNC_ENTER();
	unsigned int i;

	for (i = 0; i < (elf->symtabsz / sizeof(elf32_sym)); i++)
	{
		if (ELF32_ST_TYPE(elf->symtab[i].st_info) != 0x2)
		{
			continue;
		}

		if ((addr >= elf->symtab[i].st_value) &&
		        (addr < (elf->symtab[i].st_value + elf->symtab[i].st_size)))
		{
			const char *name = (const char *)((uint32_t)elf->strtab + elf->symtab[i].st_name);
			*offset = addr - elf->symtab[i].st_value;
			return name;
		}
	}

	*offset = -1; //invalid
	return "??????";
}

void ksymbol_init(multiboot_info_t *mbi)
{

	FUNC_ENTER();
	uint32_t fixed_addr;

	multiboot_elf_section_header_table_t *elf_sec;

	if (!(mbi->flags & MULTIBOOT_INFO_ELF_SHDR))
	{
		return;
	}

	elf_sec = &mbi->u.elf_sec;

	fixed_addr = (addr_t) mem_page_map_kernel(PAGE_ALIGN_DOWN(elf_sec->addr), divide_up(elf_sec->size, PAGE_SIZE), READ_WRITE_KERNEL);
	fixed_addr |= elf_sec->addr & ~PAGE_MASK;
	pr_debug("multiboot header: sections %u size %u addr: 0x%x -> 0x%x shndx %u\r\n",
	         elf_sec->num, elf_sec->size, elf_sec->addr, fixed_addr, elf_sec->shndx);
	elf_sec->addr = fixed_addr;

	elf_from_multiboot(elf_sec, &kernel_elf);
}

