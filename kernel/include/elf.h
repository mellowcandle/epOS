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

#ifndef ELF_H_3IBXPVEH
#define ELF_H_3IBXPVEH

#include <types.h>

typedef struct
{
	uint32_t name;
	uint32_t type;
	uint32_t flags;
	uint32_t addr;
	uint32_t offset;
	uint32_t size;
	uint32_t link;
	uint32_t info;
	uint32_t addralign;
	uint32_t entsize;
} __attribute__((packed)) elf_section_header_t;

#define ELF32_ST_TYPE(i) ((i)&0xf)

typedef struct
{
	uint32_t name;
	uint32_t value;
	uint32_t size;
	uint8_t  info;
	uint8_t  other;
	uint16_t shndx;
} __attribute__((packed)) elf_symbol_t;

typedef struct
{
	elf_symbol_t *symtab;
	uint32_t      symtabsz;
	const char   *strtab;
	uint32_t      strtabsz;
} elf_t;

// Takes a multiboot structure and returns an elf structure containing the symbol information.
//elf_t elf_from_multiboot (multiboot_t *mb);

// Looks up a symbol by address.
const char *elf_lookup_symbol(uint32_t addr, elf_t *elf, int *offset);

#endif /* end of include guard: ELF_H_3IBXPVEH */
