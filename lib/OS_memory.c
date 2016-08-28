#include "lib/OS_memory.h"

void *memset(void *s, int c, size_t n)
{
	uint8_t *p = s;

	for (unsigned int i = 0; i < n; i++)
	{
		p[i] = (uint8_t) c;
	}

	return s;
}

void *memcpy(void *restrict s1, const void *restrict s2, size_t n)
{
	const uint8_t *src = s2;
	uint8_t *dst = s1;

	while (n--)
	{
		*dst++ = *src++;
	}

	return s1;
}

int   memcmp(const void *s1, const void *s2, size_t n)
{
	const uint8_t *p1 = s1, *p2 = s2;

	while (n--)
		if (*p1 != *p2)
		{
			return *p1 - *p2;
		}
		else
		{
			*p1++, *p2++;
		}

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



void *realloc(void *ptr, size_t size)
{
// TODO: Implement
	return NULL;
}

void free(void *ptr)
{
// TODO: Implement
}

void *malloc(size_t size)
{
// TODO: Implement
	return NULL;
}

void *calloc(size_t nmemb, size_t size)
{
// TODO: Implement
	return NULL;
}
