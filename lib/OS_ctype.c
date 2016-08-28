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

#include "lib/OS_ctype.h"

char _ctmp;
unsigned char _ctype[] = {0x00,			/* EOF */
                          _C, _C, _C, _C, _C, _C, _C, _C,			/* 0-7 */
                          _C, _C | _S, _C | _S, _C | _S, _C | _S, _C | _S, _C, _C,		/* 8-15 */
                          _C, _C, _C, _C, _C, _C, _C, _C,			/* 16-23 */
                          _C, _C, _C, _C, _C, _C, _C, _C,			/* 24-31 */
                          _S | _SP, _P, _P, _P, _P, _P, _P, _P,			/* 32-39 */
                          _P, _P, _P, _P, _P, _P, _P, _P,			/* 40-47 */
                          _D, _D, _D, _D, _D, _D, _D, _D,			/* 48-55 */
                          _D, _D, _P, _P, _P, _P, _P, _P,			/* 56-63 */
                          _P, _U | _X, _U | _X, _U | _X, _U | _X, _U | _X, _U | _X, _U,	/* 64-71 */
                          _U, _U, _U, _U, _U, _U, _U, _U,			/* 72-79 */
                          _U, _U, _U, _U, _U, _U, _U, _U,			/* 80-87 */
                          _U, _U, _U, _P, _P, _P, _P, _P,			/* 88-95 */
                          _P, _L | _X, _L | _X, _L | _X, _L | _X, _L | _X, _L | _X, _L,	/* 96-103 */
                          _L, _L, _L, _L, _L, _L, _L, _L,			/* 104-111 */
                          _L, _L, _L, _L, _L, _L, _L, _L,			/* 112-119 */
                          _L, _L, _L, _P, _P, _P, _P, _C,			/* 120-127 */
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		/* 128-143 */
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		/* 144-159 */
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		/* 160-175 */
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		/* 176-191 */
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		/* 192-207 */
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		/* 208-223 */
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		/* 224-239 */
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
                         };		/* 240-255 */

