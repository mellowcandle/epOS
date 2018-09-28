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

#include <elf.h>
#include <boot/multiboot.h>
#include <mem/memory.h>
#include <kmalloc.h>
#include <printk.h>
#include <lib/string.h>
#include <bits.h>
#include <process.h>

elf_t kernel_elf;

static int elf_from_multiboot(multiboot_elf_section_header_table_t *elf_sec, elf_t *elf)
{
	FUNC_ENTER();
	unsigned int i;
	uint32_t tmp_map;

	elf32_shdr *sh = (elf32_shdr *)elf_sec->addr;
	uint32_t shstrtab = sh[elf_sec->shndx].sh_addr;

	tmp_map = (addr_t) mem_page_map_kernel_single(PAGE_ALIGN_DOWN(shstrtab), READ_WRITE_KERNEL);
	shstrtab = tmp_map | (shstrtab & ~PAGE_MASK);

	pr_debug("shstrtab mapping: 0x%x -> 0x%x\r\n", sh[elf_sec->shndx].sh_addr, shstrtab);

	for (i = 0; i < elf_sec->num; i++) {
		const char *name = (const char *)(shstrtab + sh[i].sh_name);

		if (!strcmp(name, ".strtab")) {
			elf->strtabsz = sh[i].sh_size;
			tmp_map = (addr_t) mem_page_map_kernel(PAGE_ALIGN_DOWN(sh[i].sh_addr), divide_up(elf->strtabsz,
			                                       PAGE_SIZE), READ_WRITE_KERNEL);
			elf->strtab = (char *)(tmp_map | (sh[i].sh_addr & ~PAGE_MASK));
			pr_debug("strtab maping: 0x%x -> 0x%x pages: %u\r\n", sh[i].sh_addr, (addr_t) elf->strtab,
			         divide_up(elf->strtabsz, PAGE_SIZE));
		}

		if (!strcmp(name, ".symtab")) {
			elf->symtabsz = sh[i].sh_size;
			tmp_map = (addr_t) mem_page_map_kernel(PAGE_ALIGN_DOWN(sh[i].sh_addr), divide_up(elf->symtabsz,
			                                       PAGE_SIZE), READ_WRITE_KERNEL);
			elf->symtab = (elf32_sym *)(tmp_map | (sh[i].sh_addr & ~PAGE_MASK));
			pr_debug("symtab maping: 0x%x -> 0x%x pages: %u\r\n", sh[i].sh_addr, (addr_t) elf->symtab,
			         divide_up(elf->symtabsz, PAGE_SIZE));

		}
	}

	return 0;
}

const char *elf_lookup_symbol(uint32_t addr, elf_t *elf, int *offset)
{
	FUNC_ENTER();
	unsigned int i;

	for (i = 0; i < (elf->symtabsz / sizeof(elf32_sym)); i++) {
		if (ELF32_ST_TYPE(elf->symtab[i].st_info) != 0x2)
			continue;

		if ((addr >= elf->symtab[i].st_value) &&
		    (addr < (elf->symtab[i].st_value + elf->symtab[i].st_size))) {
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
		return;

	elf_sec = &mbi->u.elf_sec;

	fixed_addr = (addr_t) mem_page_map_kernel(PAGE_ALIGN_DOWN(elf_sec->addr), divide_up(elf_sec->size,
	                PAGE_SIZE), READ_WRITE_KERNEL);
	fixed_addr |= elf_sec->addr & ~PAGE_MASK;
	pr_debug("multiboot header: sections %u size %u addr: 0x%x -> 0x%x shndx %u\r\n",
	         elf_sec->num, elf_sec->size, elf_sec->addr, fixed_addr, elf_sec->shndx);
	elf_sec->addr = fixed_addr;

	elf_from_multiboot(elf_sec, &kernel_elf);
}
static int elf_load_relocateable(task_t *task, elf32_ehdr *header)
{
	FUNC_ENTER();
	int flags;
	int ret;
	elf32_phdr *phdr;

	/* Find the program headers */
	pr_debug("Program headers count: %u\r\n", header->e_phnum);

	for (int i = 0; i < header->e_phnum; i++) {
		phdr = elf_program(header, i);
		pr_debug("Header: %u\t  Type: %u Offset: 0x%x V-addr: 0x%x P-addr: 0x%x FSize: 0x%x MSize: 0x%x Flags: %x Align: %x\r\n",
		         i,	phdr->p_type, phdr->p_offset, phdr->p_vaddr, phdr->p_vaddr, phdr->p_filesz, phdr->p_memsz,
		         phdr->p_flags, phdr->p_align);

		memblock_t *block = kmalloc(sizeof(memblock_t));

		if (!block) {
			pr_error("Out of memory\r\n");
			return -1;
		}

		flags = 0;
		block->count = divide_up(phdr->p_memsz, PAGE_SIZE);
		block->p_addr = mem_get_pages(block->count);
		block->v_addr = (void *) phdr->p_vaddr;

		if (!block->p_addr) {
			pr_error("Out of memory\r\n");
			return -1;
		}

		/* In 64bit we could use also the execute bit here... */
		if (phdr->p_flags & PF_W) {
			flags = READ_WRITE_USER;
			// That's probably the data section, let's mark the end of it, incase someone calls sbrk later.
			if (task->heap_top < block->v_addr + (block->count * PAGE_SIZE))
				task->heap_top = block->v_addr + (block->count * PAGE_SIZE);
		} else
			flags = READ_ONLY_USER;

		/* Temporarily map to allow copy */
		void *tmp = mem_page_map_kernel(block->p_addr, block->count, READ_WRITE_KERNEL | PTE_TEMPORARY);
		memset(tmp,  0, block->count * PAGE_SIZE);
		char *file_ptr = ((char *) header) + phdr->p_offset;
		memcpy(tmp, file_ptr, phdr->p_filesz);
		mem_page_unmap_multiple(tmp, block->count);

		ret = mem_pages_map_pdt_multiple(task->pdt_virt_addr, block->p_addr, block->v_addr, block->count,
		                                 flags);
		pr_debug("Mapping: 0x%x - 0x%x\r\n", (uint32_t) block->v_addr,
		         (uint32_t) block->v_addr + (block->count * PAGE_SIZE));

		if (ret)
			goto error1;

		list_add(&block->list, &task->mapped_memory_list);
	}

	/* Set EIP to the entry point of the ELF */
	task->regs.eip = header->e_entry;


	return 0;

error1:
	pr_fatal("Not implemented it yet\r\n");
	panic();

	return -1;
}

static int elf_check_validity(elf32_ehdr *header)
{
	FUNC_ENTER();

	/* Validate ELF header */
	if (memcmp(elfmag, header->e_ident, selfmag))
		return -1;

	if (header->e_ident[ei_class] != elfclass32) {
		pr_error("Unsupported ELF File Class.\r\n");
		return -1;
	}

	if (header->e_ident[ei_data] != elfdata2lsb) {
		pr_error("Unsupported ELF File byte order.\r\n");
		return false;
	}

#define EM_386		 3		/* Intel 80386 */

	if (header->e_machine != EM_386) {
		pr_error("Unsupported ELF File target.\r\n");
		return -1;
	}

	if (header->e_ident[ei_version] != ev_current) {
		pr_error("Unsupported ELF File version.\r\n");
		return -1;
	}

	if (header->e_type != et_rel && header->e_type != et_exec) {
		pr_error("Unsupported ELF File type.\r\n");
		return -1;
	}

	return 0;

}

int load_elf(task_t *task, void *addr)
{
	FUNC_ENTER();
	elf32_ehdr *header = (elf32_ehdr *) addr;


	if (elf_check_validity(header)) {
		pr_error("Can't validate ELF header\r\n");
		return -1;
	}

	switch (header->e_type) {
	case et_rel:
		return elf_load_relocateable(task, header);

	case et_exec:
		return elf_load_relocateable(task, header);

	// TODO : Implement
	default:
		return -1;
	}

	return 0;
}
