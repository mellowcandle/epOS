ENTRY(EntryPoint)
OUTPUT_FORMAT(elf32-i386)

ENTRY_BASE = 0x100000;
CODE_VIRT  = 0xC0000000;

SECTIONS {
   /* The kernel will live at 3GB + 1MB in the virtual
      address space, which will be mapped to 1MB in the
      physical address space. */
	
	. = ENTRY_BASE;

	.boot : {
	*(.multiboot)
	*(.bootcode)
	*.(bootdata)
	*.(bootstack)
	}

	. += CODE_VIRT;

   .text : AT(ADDR(.text) - 0xC0000000) {
	kernel_start = .;
	*(.text)
    *(.rodata*)
   }
	kernel_end = .;

   .data ALIGN (0x1000) : AT(ADDR(.data) - 0xC0000000) {
       *(.data)
   }

   .bss : AT(ADDR(.bss) - 0xC0000000) {
       _sbss = .;
       *(COMMON)
       *(.bss)
       _ebss = .;
   }
}
