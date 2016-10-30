/*
	This is free and unencumbered software released into the public domain.

	Anyone is free to copy, modify, publish, use, compile, sell, or
	distribute this software, either in source code form or as a compiled
	binary, for any purpose, commercial or non-commercial, and by any
	means.

	In jurisdictions that recognize copyright laws, the author or authors
	of this software dedicate any and all copyright interest in the
	software to the public domain. We make this dedication for the benefit
	of the public at large and to the detriment of our heirs and
	successors. We intend this dedication to be an overt act of
	relinquishment in perpetuity of all present and future rights to this
	software under copyright law.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
	EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
	MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
	IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
	OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
	ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
	OTHER DEALINGS IN THE SOFTWARE.

	For more information, please refer to <http://unlicense.org>
*/

#include "stdint.h"

#define PORT 0x3f8   /* COM1 */

// Write a byte out to the specified port.
static inline void outb(uint16_t port, uint8_t value)
{
	__asm volatile("outb %1, %0" : : "dN"(port), "a"(value));
}

static inline uint16_t inb(uint16_t port)
{
	uint8_t ret;
	__asm volatile("inb %1, %0" : "=a"(ret) : "dN"(port));
	return ret;
}



int serial_received()
{
	return inb(PORT + 5) & 1;
}

char read_serial()
{
	while (serial_received() == 0);

	return inb(PORT);
}

int is_transmit_empty()
{
	return inb(PORT + 5) & 0x20;
}

void write_serial(char a)
{
	while (is_transmit_empty() == 0);

	outb(PORT, a);
}

void serial_write_string(const char *string)
{
	while (*string)
	{
		write_serial(*string++);
	}
}

