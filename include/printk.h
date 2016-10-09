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

#ifndef PRINTK_H_X0OBSDIJ
#define PRINTK_H_X0OBSDIJ

#include <compiler.h>
#include <lib/varargs.h>
#include <types.h>
typedef void (* log_func)(const char *string);
void register_logger(log_func func);

int printk(const char *format, ...)
__attribute__((format(printf, 1, 2)));  /* 1=format 2=params */

int vprintk(const char *format, va_list arg);
void hex_dump(void *ptr, uint32_t len);

#ifdef DEBUG
#define FUNC_ENTER() printk("+ %s\r\n", __FUNCTION__)
#define FUNC_LEAVE() printk("- %s\r\n", __FUNCTION__)
#define pr_debug(_format, _args...)	printk("DEBUG: " _format, ##_args)

#else
#define FUNC_ENTER()
#define FUNC_LEAVE()
#define pr_debug(_format, _args...)
#endif

#define pr_fatal(_format, _args...)	printk("FATAL: " _format, ##_args)
#define pr_error(_format, _args...)	printk("ERROR: " _format, ##_args)
#define pr_warn(_format, _args...)	printk("WARN: " _format, ##_args)
#define pr_info(_format, _args...)	printk("INFO: " _format, ##_args)

#endif /* end of include guard: PRINTK_H_X0OBSDIJ */
