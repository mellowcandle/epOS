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


#ifndef KERNEL_H_M1BAK62D
#define KERNEL_H_M1BAK62D

#include <compiler.h>
#include <types.h>

/* Forward declerations */
struct multiboot_info;
typedef struct multiboot_info multiboot_info_t;

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define MAX(_x, _y) (((_x) > (_y)) ? (_x) : (_y))
#define MIN(_x, _y) (((_x) < (_y)) ? (_x) : (_y))
#define BETWEEN(a, b, c) \
	({ __typeof__ (a) __a = (a); \
	__typeof__ (b) __b = (b); \
	__typeof__ (c) __c = (c); \
	(__a >= __b && __a <= __c)|| \
	(__a >= __c && __a <= __b);})
/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:	the pointer to the member.
 * @type:	the type of the container struct this is embedded in.
 * @member:	the name of the member within the struct.
 *
 */

#define OFFSET_OF(_type, _member) ((size_t) &((_type *) 0)->_member)
#define CONTAINER_OF(_ptr, _type, _member) (_type *) ((char *) _ptr - OFFSET_OF(_type, _member))

#define assert(_x) \
	do { \
		if (!(_x)) { \
			printk("!!! assert failed %s:%u : (%s) !!!\r\n", __FILE__, __LINE__, #_x); \
			panic(); \
		} \
	} while (0)

#define mem_assert(_x) \
	do { \
		if (!(_x)) { \
			printk("!!! assert failed %s:%u : (%s) !!!\r\n", __FILE__, __LINE__, #_x); \
			dump_pdt(); \
			panic(); \
		} \
	} while (0)

/* panic.c */
void panic(void);

/* elf.c */
void ksymbol_init(multiboot_info_t *mbi);

/* ticks.c */
void ticks_init();
void sleep_ms(uint32_t delay);

#endif /* end of include guard: KERNEL_H_M1BAK62D */

