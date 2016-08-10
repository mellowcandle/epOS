#ifndef OS_TYPES_H_INCLUDED
#define OS_TYPES_H_INCLUDED

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


#endif // OS_TYPES_H_INCLUDED
