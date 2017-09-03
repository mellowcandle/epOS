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

/* type for a 16-bit quantity.  */
typedef uint16_t elf32_half;
typedef uint16_t elf64_half;

/* types for signed and unsigned 32-bit quantities.  */
typedef uint32_t elf32_word;
typedef	int32_t  elf32_sword;
typedef uint32_t elf64_word;
typedef	int32_t  elf64_sword;

/* types for signed and unsigned 64-bit quantities.  */
typedef uint64_t elf32_xword;
typedef	int64_t  elf32_sxword;
typedef uint64_t elf64_xword;
typedef	int64_t  elf64_sxword;

/* type of addresses.  */
typedef uint32_t elf32_addr;
typedef uint64_t elf64_addr;

/* type of file offsets.  */
typedef uint32_t elf32_off;
typedef uint64_t elf64_off;

/* type for section indices, which are 16-bit quantities.  */
typedef uint16_t elf32_section;
typedef uint16_t elf64_section;

/* type for version symbol information.  */
typedef elf32_half elf32_versym;
typedef elf64_half elf64_versym;


/* the elf file header.  this appears at the start of every elf file.  */

#define ei_nident (16)

typedef struct
{
  unsigned char	e_ident[ei_nident];	/* magic number and other info */
  elf32_half	e_type;			/* object file type */
  elf32_half	e_machine;		/* architecture */
  elf32_word	e_version;		/* object file version */
  elf32_addr	e_entry;		/* entry point virtual address */
  elf32_off	e_phoff;		/* program header table file offset */
  elf32_off	e_shoff;		/* section header table file offset */
  elf32_word	e_flags;		/* processor-specific flags */
  elf32_half	e_ehsize;		/* elf header size in bytes */
  elf32_half	e_phentsize;		/* program header table entry size */
  elf32_half	e_phnum;		/* program header table entry count */
  elf32_half	e_shentsize;		/* section header table entry size */
  elf32_half	e_shnum;		/* section header table entry count */
  elf32_half	e_shstrndx;		/* section header string table index */
} elf32_ehdr;

typedef struct
{
  unsigned char	e_ident[ei_nident];	/* magic number and other info */
  elf64_half	e_type;			/* object file type */
  elf64_half	e_machine;		/* architecture */
  elf64_word	e_version;		/* object file version */
  elf64_addr	e_entry;		/* entry point virtual address */
  elf64_off	e_phoff;		/* program header table file offset */
  elf64_off	e_shoff;		/* section header table file offset */
  elf64_word	e_flags;		/* processor-specific flags */
  elf64_half	e_ehsize;		/* elf header size in bytes */
  elf64_half	e_phentsize;		/* program header table entry size */
  elf64_half	e_phnum;		/* program header table entry count */
  elf64_half	e_shentsize;		/* section header table entry size */
  elf64_half	e_shnum;		/* section header table entry count */
  elf64_half	e_shstrndx;		/* section header string table index */
} elf64_ehdr;

/* fields in the e_ident array.  the ei_* macros are indices into the
   array.  the macros under each ei_* macro are the values the byte
   may have.  */

#define ei_mag0		0		/* file identification byte 0 index */
#define elfmag0		0x7f		/* magic number byte 0 */

#define ei_mag1		1		/* file identification byte 1 index */
#define elfmag1		'e'		/* magic number byte 1 */

#define ei_mag2		2		/* file identification byte 2 index */
#define elfmag2		'l'		/* magic number byte 2 */

#define ei_mag3		3		/* file identification byte 3 index */
#define elfmag3		'f'		/* magic number byte 3 */

/* conglomeration of the identification bytes, for easy testing as a word.  */
#define	elfmag		"\177elf"
#define	selfmag		4

#define ei_class	4		/* file class byte index */
#define elfclassnone	0		/* invalid class */
#define elfclass32	1		/* 32-bit objects */
#define elfclass64	2		/* 64-bit objects */
#define elfclassnum	3

#define ei_data		5		/* data encoding byte index */
#define elfdatanone	0		/* invalid data encoding */
#define elfdata2lsb	1		/* 2's complement, little endian */
#define elfdata2msb	2		/* 2's complement, big endian */
#define elfdatanum	3

#define ei_version	6		/* file version byte index */
					/* value must be ev_current */

#define ei_osabi	7		/* os abi identification */
#define elfosabi_none		0	/* unix system v abi */
#define elfosabi_sysv		0	/* alias.  */
#define elfosabi_hpux		1	/* hp-ux */
#define elfosabi_netbsd		2	/* netbsd.  */
#define elfosabi_gnu		3	/* object uses gnu elf extensions.  */
#define elfosabi_linux		elfosabi_gnu /* compatibility alias.  */
#define elfosabi_solaris	6	/* sun solaris.  */
#define elfosabi_aix		7	/* ibm aix.  */
#define elfosabi_irix		8	/* sgi irix.  */
#define elfosabi_freebsd	9	/* freebsd.  */
#define elfosabi_tru64		10	/* compaq tru64 unix.  */
#define elfosabi_modesto	11	/* novell modesto.  */
#define elfosabi_openbsd	12	/* openbsd.  */
#define elfosabi_arm_aeabi	64	/* arm eabi */
#define elfosabi_arm		97	/* arm */
#define elfosabi_standalone	255	/* standalone (embedded) application */

#define ei_abiversion	8		/* abi version */

#define ei_pad		9		/* byte index of padding bytes */

/* legal values for e_type (object file type).  */

#define et_none		0		/* no file type */
#define et_rel		1		/* relocatable file */
#define et_exec		2		/* executable file */
#define et_dyn		3		/* shared object file */
#define et_core		4		/* core file */
#define	et_num		5		/* number of defined types */
#define et_loos		0xfe00		/* os-specific range start */
#define et_hios		0xfeff		/* os-specific range end */
#define et_loproc	0xff00		/* processor-specific range start */
#define et_hiproc	0xffff		/* processor-specific range end */

/* if it is necessary to assign new unofficial em_* values, please
   pick large random numbers (0x8523, 0xa7f2, etc.) to minimize the
   chances of collision with official or non-gnu unofficial values.  */

#define em_alpha	0x9026

/* legal values for e_version (version).  */

#define ev_none		0		/* invalid elf version */
#define ev_current	1		/* current version */
#define ev_num		2

/* section header.  */

typedef struct
{
  elf32_word	sh_name;		/* section name (string tbl index) */
  elf32_word	sh_type;		/* section type */
  elf32_word	sh_flags;		/* section flags */
  elf32_addr	sh_addr;		/* section virtual addr at execution */
  elf32_off	sh_offset;		/* section file offset */
  elf32_word	sh_size;		/* section size in bytes */
  elf32_word	sh_link;		/* link to another section */
  elf32_word	sh_info;		/* additional section information */
  elf32_word	sh_addralign;		/* section alignment */
  elf32_word	sh_entsize;		/* entry size if section holds table */
} elf32_shdr;

#define ELF32_ST_TYPE(i) ((i)&0xf)

/* symbol table entry.  */

typedef struct
{
  elf32_word	st_name;		/* symbol name (string tbl index) */
  elf32_addr	st_value;		/* symbol value */
  elf32_word	st_size;		/* symbol size */
  unsigned char	st_info;		/* symbol type and binding */
  unsigned char	st_other;		/* symbol visibility */
  elf32_section	st_shndx;		/* section index */
} elf32_sym;


typedef struct
{
	elf32_sym *symtab;
	uint32_t      symtabsz;
	const char   *strtab;
	uint32_t      strtabsz;
} elf_t;

// Takes a multiboot structure and returns an elf structure containing the symbol information.
//elf_t elf_from_multiboot (multiboot_t *mb);

// Looks up a symbol by address.
const char *elf_lookup_symbol(uint32_t addr, elf_t *elf, int *offset);

struct __task_t;
int load_elf(struct __task_t *task, void *addr);

#endif /* end of include guard: ELF_H_3IBXPVEH */
