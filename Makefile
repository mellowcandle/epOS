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

.PHONY: all clean dist modules cscope multiboot style lines prepare libc

CWD = $(shell pwd)
export TOOLCHAIN_PATH = $(CWD)/toolchain/i686-elf-4.9.1-Linux-x86_64
export CC	= $(TOOLCHAIN_PATH)/bin/i686-elf-gcc
export LD	= $(TOOLCHAIN_PATH)/bin/i686-elf-ld
export AR	= $(TOOLCHAIN_PATH)/bin/i686-elf-ar
export ASM	= nasm
export WARNINGS := -Wall -Wextra -Wno-unused-value -Wno-unused-parameter -Wno-char-subscripts

ISODIR_PATH = isodir

ASTYLE ?= astyle
ASTYLE_CONFIG := --suffix=none --style=allman --indent=tab --indent-classes --indent-namespaces --pad-oper --pad-header \
	--add-brackets --align-pointer=name --align-reference=name --lineend=linux --break-blocks --unpad-paren

all: kernel.iso cscope

prepare:
	@tar xvf toolchain/i686-elf-4.9.1-Linux-x86_64.tar.xz -C toolchain

modules:
	@echo "Building multiboot modules:"
	@$(ASM) -f bin modules/program.s -o modules/program

kernel.iso: multiboot
	@rm -rf $(ISODIR_PATH)
	@mkdir -p $(ISODIR_PATH)/boot/grub
	@mkdir -p $(ISODIR_PATH)/modules
	@cp kernel/kernel.bin $(ISODIR_PATH)/boot/kernel.bin
	@cp make/grub.cfg $(ISODIR_PATH)/boot/grub/grub.cfg
	@cp modules/program $(ISODIR_PATH)/modules
	@grub-mkrescue -o kernel.iso $(ISODIR_PATH) > /dev/null 2>&1

cscope:
	@echo "Preparing cscope tags"
	@find . \( -name *.[csh] \) -and -not \( -path "./toolchain/*" \) > cscope.files
	@cscope -b -q -k

style:
	@find . \( -name *.[ch] \) -and -not \( -path "./toolchain/*" \) | xargs ${ASTYLE} ${ASTYLE_CONFIG}

lines:
	@cloc --exclude-dir=toolchain,kernel/drivers/acpi/acpica --exclude-lang=XML,D,Markdown,make,Python,DTD .

multiboot:
	@$(MAKE) -C kernel

libc:
	@$(MAKE) -C libc

clean:
	@$(MAKE) -C kernel clean
	@$(MAKE) -C libc clean


