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

#include "OS_types.h"

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

/*
The calloc function allocates space for an array of nmemb objects, each of whose size is size.
The space is initialized to all bits zero.238)
The calloc function returns either a null pointer or a pointer to the allocated space.
*/
void *calloc(size_t nmemb, size_t size);

/*
The malloc function allocates space for an object whose size is specified by size and whose value is indeterminate.
The malloc function returns either a null pointer or a pointer to the allocated space.
*/
void *malloc(size_t size);

/*
The free function causes the space pointed to by ptr to be deallocated, that is,
made available for further allocation. If ptr is a null pointer, no action occurs. Otherwise,
if the argument does not match a pointer earlier returned by the calloc, malloc, or realloc function,
or if the space has been deallocated by a call to free or realloc, the behavior is undefined.
The free function returns no value.
*/
void free(void *ptr);

/*
The realloc function deallocates the old object pointed to by ptr and returns a pointer to a new object
that has the size specified by size. The contents of the new object shall be the same as that
of the old object prior to deallocation, up to the lesser of the new and old sizes.
Any bytes in the new object beyond the size of the old object have indeterminate values.
If ptr is a null pointer, the realloc function behaves like the malloc function for the specified size.
Otherwise, if ptr does not match a pointer earlier returned by the calloc, malloc, or realloc function,
or if the space has been deallocated by a call to the free or realloc function,
the behavior is undefined. If memory for the new object cannot be allocated,
the old object is not deallocated and its value is unchanged.

The realloc function returns a pointer to the new object (which may have the same value as a pointer to the old object),
or a null pointer if the new object could not be allocated.
*/
void *realloc(void *ptr, size_t size);


