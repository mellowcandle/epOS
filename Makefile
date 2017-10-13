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

.PHONY: all clean dist apps overlay modules cscope multiboot style lines
ifndef EPOS_ROOTDIR
$(error EPOS_ROOTDIR is not defined, did you run ./prepare ? did you source environment ???)
endif

# Determine core count so we can hurry up build
NPROCS:=1
OS:=$(shell uname -s)

ifeq ($(OS),Linux)
	NPROCS:=$(shell grep -c ^processor /proc/cpuinfo)
endif
ifeq ($(OS),Darwin) # Assume Mac OS X
	NPROCS:=$(shell system_profiler | awk '/Number Of CPUs/{print $4}{next;}')
endif

ISODIR_PATH = isodir

ASTYLE ?= astyle
ASTYLE_CONFIG := --suffix=none --style=allman --indent=tab --indent-classes --indent-namespaces --pad-oper --pad-header \
	--add-brackets --align-pointer=name --align-reference=name --lineend=linux --break-blocks --unpad-paren

all: kernel.iso apps cscope overlay

apps:
	@echo "Building Applications:"
	@$(MAKE) --no-print-directory -j$(NPROCS) -C apps

kernel.iso: multiboot overlay
	@rm -rf $(ISODIR_PATH)
	@mkdir -p $(ISODIR_PATH)/boot/grub
	@mkdir -p $(ISODIR_PATH)/modules
	@cp kernel/kernel.bin $(ISODIR_PATH)/boot/kernel.bin
	@cp rootfs.tar $(ISODIR_PATH)/boot/rootfs.tar
	@cp make/grub.cfg $(ISODIR_PATH)/boot/grub/grub.cfg
	@grub-mkrescue -o kernel.iso $(ISODIR_PATH) > /dev/null 2>&1

cscope:
	@echo "Preparing cscope tags"
	@find . \( -name *.[csh] \) -and -not \( -path "./toolchain/*" \) -and -not \( -path "./3rd_party/*" \) -and -not \( -path "./sysroot/*" \) > cscope.files
	@cscope -b -q -k

style:
	@find . \( -name *.[ch] \) -and -not \( -path "./toolchain/*" \) -and -not \( -path "./3rd_party/*" \) -and -not \( -path "./sysroot/*" \) | xargs ${ASTYLE} ${ASTYLE_CONFIG}

lines:
	@cloc --exclude-dir=toolchain,kernel/drivers/acpi/acpica,3rd_party,overlay --exclude-lang=XML,D,Markdown,make,Python,DTD .

multiboot:
	@$(MAKE) --no-print-directory -j$(NPROCS) -C kernel

clean:
	@rm -rf rootfs.tar
	@rm -rf $(ISODIR_PATH)
	@$(MAKE) --no-print-directory -C kernel clean
	@$(MAKE) --no-print-directory -C apps clean

overlay: apps
	@echo "Preparing RAMFS:"
	@mkdir -p overlay/bin
	@cp apps/test1 overlay/bin/test1
#	@cp apps/test2 overlay/bin/test2
#	@tar cvf rootfs.tar -C ${EPOS_SYSROOT} .
	@tar cvf rootfs.tar -C overlay .
