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

#include <lib/string.h>

void *memset(void *s, int c, size_t n)
{
	uint8_t *p = s;

	for (unsigned int i = 0; i < n; i++)
		p[i] = (uint8_t) c;

	return s;
}

void *memcpy(void *restrict s1, const void *restrict s2, size_t n)
{
	const uint8_t *src = s2;
	uint8_t *dst = s1;

	while (n--)
		*dst++ = *src++;

	return s1;
}

int   memcmp(const void *s1, const void *s2, size_t n)
{
	const uint8_t *p1 = s1, *p2 = s2;

	while (n--)
		if (*p1 != *p2)
			return *p1 - *p2;
		else
			p1++, p2++;

	return 0;
}

void *memmove(void *s1, const void *s2, size_t n)
{
// this only work on c99, VLA needed.
	unsigned char tmp[n];
	memcpy(tmp, s2, n);
	memcpy(s1, tmp, n);
	return s1;

}

size_t strlen(const char *str)
{
	size_t len = 0;

	while (*str++)
		len++;

	return len;
}

int strcmp(const char *s1, const char *s2)
{
	for (; *s1 == *s2; s1++, s2++)
		if (*s1 == '\0')
			return 0;

	return ((*(unsigned char *)s1 < * (unsigned char *)s2) ? -1 : +1);
}

int strncmp(const char *s1, const char *s2, size_t n)
{
	for (; n > 0; s1++, s2++, --n)
		if (*s1 != *s2)
			return ((*(unsigned char *)s1 < * (unsigned char *)s2) ? -1 : +1);
		else if (*s1 == '\0')
			return 0;

	return 0;
}

char *strcpy(char *restrict s1, const char *restrict s2)
{
	char *s = s1;

	while ((*s++ = *s2++) != 0)
		;

	return (s1);
}

char *strncpy(char *restrict s1, const char *restrict s2, size_t n)
{
	char *s = s1;

	while (n > 0 && *s2 != '\0') {
		*s++ = *s2++;
		--n;
	}

	while (n > 0) {
		*s++ = '\0';
		--n;
	}

	return s1;
}

char *strcat(char *restrict s1, const char *restrict s2)
{
	strcpy(&s1[strlen(s1)], s2);
	return s1;
}

