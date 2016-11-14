# This is free and unencumbered software released into the public domain.
#
# Anyone is free to copy, modify, publish, use, compile, sell, or
# distribute this software, either in source code form or as a compiled
# binary, for any purpose, commercial or non-commercial, and by any
# means.
#
# In jurisdictions that recognize copyright laws, the author or authors
# of this software dedicate any and all copyright interest in the
# software to the public domain. We make this dedication for the benefit
# of the public at large and to the detriment of our heirs and
# successors. We intend this dedication to be an overt act of
# relinquishment in perpetuity of all present and future rights to this
# software under copyright law.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.
#
# For more information, please refer to <http://unlicense.org>

.PHONY: program

TOOLCHAIN_PATH = ../toolchain/i686-elf-4.9.1-Linux-x86_64
CC	= $(TOOLCHAIN_PATH)/bin/i686-elf-gcc
LD	= $(TOOLCHAIN_PATH)/bin/i686-elf-ld
ASM	= nasm
WARNINGS := -Wall -Wextra -Wno-unused-value -Wno-unused-parameter
CFLAGS := -std=gnu99  -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -c
INCLUDES := -I../libc
all: program

OBJFILES := program.o

.c.o:
	@echo "(CC) $@"
	@$(CC) $(CFLAGS) $(INCLUDES) $< -o $@

.s.o:
	@echo "(AS) $@"
	@$(ASM) -f elf -o $@ $<

program: $(OBJFILES)

	$(CC) -T linker.ld -Wl,-Map=program.map -L../libc -ffreestanding -nostdlib -lc -o $@ $^ ../libc/libc.a $(TOOLCHAIN_PATH)/lib/gcc/i686-elf/4.9.1/libgcc.a # emulate 32 bits ELF, the binary output is specified

clean:
	rm -rf $(OBJFILES) program program.map
