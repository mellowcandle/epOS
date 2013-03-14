# Make file for epOS by Ramon Fried
# Last modifcation: 21/1/2013

.PHONY: all clean dist check testdrivers todolist



CC	= x86_64-elf-gcc
LD	= x86_64-elf-ld
ASM	= nasm
WARNINGS := -Wall -Wextra -pedantic -Wshadow -Wpointer-arith -Wcast-align \
            -Wwrite-strings -Wmissing-prototypes -Wmissing-declarations \
            -Wredundant-decls -Wnested-externs -Winline -Wno-long-long \
            -Wuninitialized -Wconversion -Wstrict-prototypes
CFLAGS := -g -std=c99 $(WARNINGS)

PROJDIRS := inc src tests

SRCFILES := $(shell find $(PROJDIRS) -type f -name "\*.c")
SRCFILES += $(shell find $(PROJDIRS) -type f -name "\*.s")

HDRFILES := $(shell find $(PROJDIRS) -type f -name "\*.h")

OBJFILES := $(patsubst %.c,%.o,$(SRCFILES))
OBJFILES += $(patsubst %.s,%.o,$(SRCFILES))

TSTFILES := $(patsubst %.c,%_t,$(SRCFILES))

all: kernel.img

kernel.bin: $(OBJFILES)
	$(LD) -T make/linker.ld -o $@ $^

kernel.img: kernel.bin
	dd if=/dev/zero of=pad bs=1 count=750
	cat stage1 stage2 pad $< > $@
clean:

%.o: %.c Makefile
	@$(CC) $(CFLAGS) -MMD -MP -c $< -o $@
%o: %.s
	nasm -f elf -o $@ $<



