# Make file for epOS by Ramon Fried
# Last modifcation: 21/1/2013

.PHONY: all clean dist check testdrivers todolist

WARNINGS := -Wall -Wextra -pedantic -Wshadow -Wpointer-arith -Wcast-align \
            -Wwrite-strings -Wmissing-prototypes -Wmissing-declarations \
            -Wredundant-decls -Wnested-externs -Winline -Wno-long-long \
            -Wuninitialized -Wconversion -Wstrict-prototypes
CFLAGS := -g -std=c99 $(WARNINGS)

PROJDIRS := inc src tests

SRCFILES := $(shell find $(PROJDIRS) -type f -name "\*.c")
HDRFILES := $(shell find $(PROJDIRS) -type f -name "\*.h")

OBJFILES := $(patsubst %.c,%.o,$(SRCFILES))
TSTFILES := $(patsubst %.c,%_t,$(SRCFILES))

kernel.o: kernel.c

loader.o: loader.s

ports.o: ports.c 

ctype.o: ctype.c

OS_memory.o: OS_memory.c

VIDEO_textmode.o: VIDEO_testmode.c





