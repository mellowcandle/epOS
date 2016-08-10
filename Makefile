# Make file for epOS by Ramon Fried

.PHONY: all clean clean_dep dist check testdrivers todolist cscope cscope_update multiboot

TOOLCHAIN_PATH = toolchain/i686-elf-4.9.1-Linux-x86_64/bin
CC	= $(TOOLCHAIN_PATH)/i686-elf-gcc
LD	= $(TOOLCHAIN_PATH)/i686-elf-ld
ASM	= nasm
WARNINGS := -Wall -Wextra 

CFLAGS := -g -std=gnu99 -nostdlib -nostartfiles -nodefaultlibs $(WARNINGS)

PROJDIRS := include kernel lib boot drivers

INCLUDES := -Iinclude
SRCFILES := $(shell find $(PROJDIRS) -type f -name "*.c")
SRCFILES += $(shell find $(PROJDIRS) -type f -name "*.s")

OBJFILES := $(patsubst %.c,%.o,$(SRCFILES))
OBJFILES := $(patsubst %.s,%.o,$(OBJFILES))
DEPFILES := $(OBJFILES)

-include $(dep)   # include all dep files in the makefile

%.d: %.c
	@$(CC) $(CFLAGS) $< -MM -MT $(@:.d=.o) >$@

all: kernel.iso

test:
	@echo "Source files: "
	@echo $(SRCFILES)
	@echo "Object files: "
	@echo $(OBJFILES)
#kernel.bin: src/kernel/kernel.o src/kernel/loader.o
kernel.bin: $(OBJFILES)
	$(LD) -T make/linker.ld -o $@ $^


multiboot: kernel.bin
	@echo "Verifying multiboot:"
	@grub-file --is-x86-multiboot kernel.bin ; if [ $$? -eq 1 ] ; then echo "Error: Multiboot missing/corrupted.";exit 1; fi
	@echo "Multiboot header detected."


kernel.iso: multiboot
	@rm -rf isodir
	@mkdir -p isodir/boot/grub
	@cp kernel.bin isodir/boot/kernel.bin
	@cp make/grub.cfg isodir/boot/grub/grub.cfg
	@grub-mkrescue -o kernel.iso isodir
	
clean:
	$(RM) $(OBJFILES) kernel.bin kernel.img

cleandep:
	@rm -f $(dep)
.c.o:
	@$(CC) $(CFLAGS) $(INCLUDES) -MMD -MP -c $< -o $@
	
.s.o:
	@$(ASM) -f elf -o $@ $<

cscope:
	@echo "Preparing cscope tags"
	@find $(PROJDIRS) -name *.[csh] > cscope.files
	@cscope -b -q -k

cscope_update:
	@echo "Updating cscope tags"
	@cscope -b -q -k
