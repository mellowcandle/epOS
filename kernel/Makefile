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

.PHONY: all clean dist modules testdrivers todolist cscope cscope_update multiboot style lines prepare

WARNINGS := -Wall -Wextra -Wno-unused-value -Wno-unused-parameter -Wno-char-subscripts -Wno-missing-field-initializers
CWD = $(shell pwd)
TOOLCHAIN_PATH = $(CWD)/../toolchain/i686-elf-7.2.0-epos-x86_64/
CC      = $(TOOLCHAIN_PATH)/bin/i686-epos-gcc
LD      = $(TOOLCHAIN_PATH)/bin/i686-epos-ld
AR      = $(TOOLCHAIN_PATH)/bin/i686-epos-ar
ASM     = nasm

CFLAGS := -g -std=gnu99 -nostdlib -ffreestanding -fstack-protector-strong $(WARNINGS)

PROJDIRS := include kernel lib boot drivers mem arch

INCLUDES := -Iinclude -Iarch/x86/include -Idrivers/include -Iinclude/uapi
SRCFILES := $(shell find $(PROJDIRS) -type f -name "*.c")
SRCFILES += $(shell find $(PROJDIRS) -type f -name "*.s")

OBJFILES := $(patsubst %.c,%.o,$(SRCFILES))
OBJFILES := $(patsubst %.s,%.o,$(OBJFILES))
DEPFILES := $(patsubst %.o,%.d,$(OBJFILES))
include $(dep)   # include all dep files in the makefile

%.d: %.c
	@$(CC) $(CFLAGS) $< -MM -MT $(@:.d=.o) >$@

all: multiboot

kernel.bin: $(OBJFILES)
	@$(CC) -L${TOOLCHAIN_PATH}/lib/gcc/i686-elf/4.9.1/ -T linker.ld -Wl,-Map=kernel.map -ffreestanding -nostdlib -o $@ $^ -lgcc


multiboot: kernel.bin
	@echo "Verifying multiboot:"
	@grub-file --is-x86-multiboot kernel.bin ; if [ $$? -eq 1 ] ; then echo "Error: Multiboot missing/corrupted.";exit 1; fi
	@echo "Multiboot header detected."


clean:
	$(RM) $(OBJFILES) $(DEPFILES) kernel.bin kernel.map

.c.o:
	@echo "EPOS - (CC) $@"
	@$(CC) $(CFLAGS) $(INCLUDES) -MMD -MP -c $< -o $@

.s.o:
	@echo "EPOS - (AS) $@"
	@$(ASM) -f elf -o $@ $<

