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

#ifndef BITS_H_W0QIETBW
#define BITS_H_W0QIETBW

#include <types.h>

#define BIT(_x)			(1 << (_x))
#define BIT_CHECK(a,b)	((a) & (1<<(b)))
#define BIT_SET(a,b)	((a) |= (1<<(b)))
#define BIT_CLEAR(a,b)	((a) &= ~(1<<(b)))
#define BIT_TOGGLE(a,b) ((a) ^= (1<<(b)))

/* Some macros taken from: http://www.coranac.com/documents/working-with-bits-and-bitfields/ */

#define BIT_MASK(len)			(BIT(len) - 1)
#define BF_MASK(start, len)		(BIT_MASK(len) << (start))
#define BF_PREP(x, start, len)  (((x)&BIT_MASK(len)) << (start))

//Extract a bitfield of length \a len starting at bit \a start from \a y.
#define BF_GET(y, start, len)   (((y) >> (start)) & BIT_MASK(len))

//! Insert a new bitfield value \a x into \a y.
#define BF_SET(y, x, start, len) \
	(y = ((y) & ~BF_MASK(start, len)) | BF_PREP(x, start, len))

/*
 *  Determine whether some value is a power of two, where zero is
 * *not* considered a power of two.
 */

static inline __attribute__((const))
bool is_power_of_2(unsigned long n)
{
	return (n != 0 && ((n & (n - 1)) == 0));
}

static inline uint32_t divide_up(uint32_t value, uint32_t align)
{
	uint32_t ret;
	ret = value / align;

	if (value % align)
	{
		ret++;
	}

	return ret;
}

#endif /* end of include guard: BITS_H_W0QIETBW */


