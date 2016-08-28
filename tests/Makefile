# Make file for epOS by Ramon Fried

.PHONY: all clean tests 

WARNINGS := -Wall -Wextra 

CFLAGS := -g -std=gnu99 $(WARNINGS)


SRCFILES := tests.c ../lib/printk.c
OBJFILES := $(patsubst %.c,%.o,$(SRCFILES))

INCLUDES := -I../include



all: test_kernel


test_kernel: $(OBJFILES)
	$(CC) -lcheck -o $@ $^

clean:
	$(RM) $(OBJFILES) test_kernel

.c.o:
	@$(CC) $(CFLAGS) $(INCLUDES) -MMD -MP -c $< -o $@
	
