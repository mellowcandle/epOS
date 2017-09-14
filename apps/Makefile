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

.PHONY: test1 test2

CC	= $(TOOLCHAIN_PATH)/bin/i686-epos-gcc
LD	= $(TOOLCHAIN_PATH)/bin/i686-epos-ld
ASM	= nasm
WARNINGS := -Wall -Wextra -Wno-unused-value -Wno-unused-parameter
CFLAGS := -std=gnu99  -fno-stack-protector -c -g --sysroot=${EPOS_SYSROOT}
INCLUDES := -I../libc -I../kernel/include/uapi

all: test1

OBJFILES := test1.o
OBJFILES2 := test2.o

.c.o:
	@echo "(CC) $@"
	@$(CC) $(CFLAGS) $(INCLUDES) $< -o $@

.s.o:
	@echo "(AS) $@"
	@$(ASM) -f elf -o $@ $<

test1: $(OBJFILES)
	$(CC) --sysroot=${EPOS_SYSROOT} -Wl,-Map=program.map  -o $@ $^  # emulate 32 bits ELF, the binary output is specified
test2: $(OBJFILES2)
	$(CC) -T linker.ld -Wl,-Map=program.map -L../libc -ffreestanding -nostdlib -lc -o $@ $^ ../libc/libc.a $(TOOLCHAIN_PATH)/lib/gcc/i686-elf/4.9.1/libgcc.a # emulate 32 bits ELF, the binary output is specified


clean:
	rm -rf $(OBJFILES) test1 test2

