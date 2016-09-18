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

#include <cpu.h>
#include <types.h>

extern void gdt_flush();

/* Defines a GDT entry. We say packed, because it prevents the
*  compiler from doing things that it thinks is best: Prevent
*  compiler "optimization" by packing */
struct gdt_entry
{
	unsigned short limit_low;
	unsigned short base_low;
	unsigned char base_middle;
	unsigned char access;
	unsigned char granularity;
	unsigned char base_high;
} __attribute__((packed));

/* Special pointer which includes the limit: The max bytes
*  taken up by the GDT, minus 1. Again, this NEEDS to be packed */
struct gdt_ptr
{
	unsigned short limit;
	unsigned int base;
} __attribute__((packed));

/* Our GDT, with 3 entries, and finally our special GDT pointer */
struct gdt_entry gdt[5];
struct gdt_ptr gp;


/* Setup a descriptor in the Global Descriptor Table */
static void gdt_set_gate(int num, unsigned long base, unsigned long limit, unsigned char access, unsigned char gran)
{
	/* Setup the descriptor base address */
	gdt[num].base_low = (base & 0xFFFF);
	gdt[num].base_middle = (base >> 16) & 0xFF;
	gdt[num].base_high = (base >> 24) & 0xFF;

	/* Setup the descriptor limits */
	gdt[num].limit_low = (limit & 0xFFFF);
	gdt[num].granularity = ((limit >> 16) & 0x0F);

	/* Finally, set up the granularity and access flags */
	gdt[num].granularity |= (gran & 0xF0);
	gdt[num].access = access;
}

/* Should be called by main. This will setup the special GDT
*  pointer, set up the first 3 entries in our GDT, and then
*  finally call gdt_flush() in our assembler file in order
*  to tell the processor where the new GDT is and update the
*  new segment registers */
void gdt_init()
{
	/* Setup the GDT pointer and limit */
	gp.limit = (sizeof(struct gdt_entry) * 5) - 1;
	gp.base = (uint32_t) &gdt;

	/* Our NULL descriptor */
	gdt_set_gate(0, 0, 0, 0, 0);

	/* The second entry is the kernel Code Segment. The base address
	*  is 0, the limit is 4GBytes, it uses 4KByte granularity,
	*  uses 32-bit opcodes, and is a Code Segment descriptor.
	*  Please check the table above in the tutorial in order
	*  to see exactly what each value means */
	gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

	/* The third entry is the kernel Data Segment. It's EXACTLY the
	*  same as our code segment, but the descriptor type in
	*  this entry's access byte says it's a Data Segment */
	gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

	/* The third entry is the user Code Segment. The base address
	*  is 0, the limit is 4GBytes, it uses 4KByte granularity,
	*  uses 32-bit opcodes, and is a Code Segment descriptor.
	*  Please check the table above in the tutorial in order
	*  to see exactly what each value means */
	gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);

	/* The third entry is the user Data Segment. It's EXACTLY the
	*  same as our code segment, but the descriptor type in
	*  this entry's access byte says it's a Data Segment */
	gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);
	/* Flush out the old GDT and install the new changes! */
	gdt_flush();
}
