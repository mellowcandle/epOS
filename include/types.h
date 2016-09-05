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

#ifndef OS_TYPES_H_VEFJ0AZY
#define OS_TYPES_H_VEFJ0AZY

typedef unsigned long long uint64_t;

typedef unsigned int uint32_t;
typedef int           int32_t;

typedef unsigned char uint8_t;
typedef char           int8_t;

typedef unsigned short uint16_t;
typedef short           int16_t;

typedef uint32_t    size_t;

typedef int             intptr_t;
typedef unsigned int   uintptr_t;
#define NULL   ( ( void * ) 0 )

#define bool    _Bool
#define true    1
#define false   0

#endif /* end of include guard: OS_TYPES_H_VEFJ0AZY */

