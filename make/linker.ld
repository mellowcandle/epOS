OUTPUT_FORMAT("elf32-i386") 
ENTRY(loader)

SECTIONS {
  .text 0x0100000 : {
    kernel_start = .;
    *(.multiboot)
    *(.text)
    . = ALIGN(4096);
  }

  .data : {
    data = .;
    *(.data)
    . = ALIGN(4096);
  }

  .bss : {
    bss = .;
    *(.bss)
    . = ALIGN(4096);
  }

  kernel_end = .;
}

