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

#include <types.h>

/*
The memmove function copies n characters from the object pointed to by s2 into the object pointed to by s1. Copying takes place as if the n characters from the object pointed to by s2 are first copied into a temporary array of n characters that does not overlap the objects pointed to by s1 and s2, and then the n characters from the temporary array are copied into the object pointed to by s1.
The memmove function returns the value of s1.
*/

void *memmove(void *s1, const void *s2, size_t n);

/*
The memset function copies the value of c (converted to an unsigned char) into each of the first n characters of the object pointed to by s.
The memset function returns the value of s.
*/
void *memset(void *s, int c, size_t n);

/*
The memcpy function copies n characters from the object pointed to by s2 into the object pointed to by s1. If copying takes place between objects that overlap, the behavior is undefined.
The memcpy function returns the value of s1.
*/
void *memcpy(void *restrict s1, const void *restrict s2, size_t n);

/*
The memcmp function compares the first n characters of the object pointed to by s1 to the first n
characters of the object pointed to by s2.248)
The memcmp function returns an integer greater than,
equal to, or less than zero, accordingly as the
object pointed to by s1 is greater than, equal to, or less than the object pointed to by s2.
*/
int   memcmp(const void *s1, const void *s2, size_t n);

size_t strlen ( const char * str );

int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
char *strcpy(char *restrict s1, const char *restrict s2);
char * strncpy(char *restrict s1, const char *restrict s2, size_t n);
char * strcat(char *restrict s1, const char *restrict s2);


