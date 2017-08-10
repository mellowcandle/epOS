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

.PHONY: all clean

SRCFILES := $(shell find . -type f -name "*.c")
SRCFILES += $(shell find . -type f -name "*.s")

OBJFILES := $(patsubst %.c,%.o,$(SRCFILES))
OBJFILES := $(patsubst %.s,%.o,$(OBJFILES))
DEPFILES := $(patsubst %.o,%.d,$(OBJFILES))

WARNINGS := -Wall -Wextra -Wno-unused-value -Wno-unused-parameter -Wno-char-subscripts
CFLAGS := -g -std=gnu99 -nostdlib -ffreestanding $(WARNINGS)
INCLUDES := -I../kernel/include/uapi

-include $(dep)   # include all dep files in the makefile

all: $(OBJFILES)
	$(AR) rcs libc.a $(OBJFILES)

%.d: %.c
	@$(CC) $(CFLAGS) $< -MM -MT $(@:.d=.o) >$@

.c.o:
	@echo "LIBC - (CC) $@"
	@$(CC) $(CFLAGS) $(INCLUDES) -MMD -MP -c $< -o $@

.s.o:
	@echo "LIBC - (AS) $@"
	@$(ASM) -f elf -o $@ $<

clean:
	$(RM) $(OBJFILES) $(DEPFILES) libc.a
