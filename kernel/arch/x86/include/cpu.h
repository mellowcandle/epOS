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

#ifndef CPU_H_
#define CPU_H_

#include <types.h>

#define CPUID_VENDOR_INTEL        "GenuineIntel"

enum {
	CPUID_FEAT_ECX_SSE3         = 1 << 0,
	CPUID_FEAT_ECX_PCLMUL       = 1 << 1,
	CPUID_FEAT_ECX_DTES64       = 1 << 2,
	CPUID_FEAT_ECX_MONITOR      = 1 << 3,
	CPUID_FEAT_ECX_DS_CPL       = 1 << 4,
	CPUID_FEAT_ECX_VMX          = 1 << 5,
	CPUID_FEAT_ECX_SMX          = 1 << 6,
	CPUID_FEAT_ECX_EST          = 1 << 7,
	CPUID_FEAT_ECX_TM2          = 1 << 8,
	CPUID_FEAT_ECX_SSSE3        = 1 << 9,
	CPUID_FEAT_ECX_CID          = 1 << 10,
	CPUID_FEAT_ECX_FMA          = 1 << 12,
	CPUID_FEAT_ECX_CX16         = 1 << 13,
	CPUID_FEAT_ECX_ETPRD        = 1 << 14,
	CPUID_FEAT_ECX_PDCM         = 1 << 15,
	CPUID_FEAT_ECX_DCA          = 1 << 18,
	CPUID_FEAT_ECX_SSE4_1       = 1 << 19,
	CPUID_FEAT_ECX_SSE4_2       = 1 << 20,
	CPUID_FEAT_ECX_x2APIC       = 1 << 21,
	CPUID_FEAT_ECX_MOVBE        = 1 << 22,
	CPUID_FEAT_ECX_POPCNT       = 1 << 23,
	CPUID_FEAT_ECX_AES          = 1 << 25,
	CPUID_FEAT_ECX_XSAVE        = 1 << 26,
	CPUID_FEAT_ECX_OSXSAVE      = 1 << 27,
	CPUID_FEAT_ECX_AVX          = 1 << 28,

	CPUID_FEAT_EDX_FPU          = 1 << 0,
	CPUID_FEAT_EDX_VME          = 1 << 1,
	CPUID_FEAT_EDX_DE           = 1 << 2,
	CPUID_FEAT_EDX_PSE          = 1 << 3,
	CPUID_FEAT_EDX_TSC          = 1 << 4,
	CPUID_FEAT_EDX_MSR          = 1 << 5,
	CPUID_FEAT_EDX_PAE          = 1 << 6,
	CPUID_FEAT_EDX_MCE          = 1 << 7,
	CPUID_FEAT_EDX_CX8          = 1 << 8,
	CPUID_FEAT_EDX_APIC         = 1 << 9,
	CPUID_FEAT_EDX_SEP          = 1 << 11,
	CPUID_FEAT_EDX_MTRR         = 1 << 12,
	CPUID_FEAT_EDX_PGE          = 1 << 13,
	CPUID_FEAT_EDX_MCA          = 1 << 14,
	CPUID_FEAT_EDX_CMOV         = 1 << 15,
	CPUID_FEAT_EDX_PAT          = 1 << 16,
	CPUID_FEAT_EDX_PSE36        = 1 << 17,
	CPUID_FEAT_EDX_PSN          = 1 << 18,
	CPUID_FEAT_EDX_CLF          = 1 << 19,
	CPUID_FEAT_EDX_DTES         = 1 << 21,
	CPUID_FEAT_EDX_ACPI         = 1 << 22,
	CPUID_FEAT_EDX_MMX          = 1 << 23,
	CPUID_FEAT_EDX_FXSR         = 1 << 24,
	CPUID_FEAT_EDX_SSE          = 1 << 25,
	CPUID_FEAT_EDX_SSE2         = 1 << 26,
	CPUID_FEAT_EDX_SS           = 1 << 27,
	CPUID_FEAT_EDX_HTT          = 1 << 28,
	CPUID_FEAT_EDX_TM1          = 1 << 29,
	CPUID_FEAT_EDX_IA64         = 1 << 30,
	CPUID_FEAT_EDX_PBE          = 1 << 31
};

enum cpuid_requests {
	CPUID_GETVENDORSTRING,
	CPUID_GETFEATURES,
	CPUID_GETTLB,
	CPUID_GETSERIAL,

	CPUID_INTELEXTENDED = 0x80000000,
	CPUID_INTELFEATURES,
	CPUID_INTELBRANDSTRING,
	CPUID_INTELBRANDSTRINGMORE,
	CPUID_INTELBRANDSTRINGEND,
};


void gdt_init();
void idt_init();

void cpu_init();


/** issue a single request to CPUID. Fits 'intel features', for instance
 *  *  note that even if only "eax" and "edx" are of interest, other registers
 *   *  will be modified by the operation, so we need to tell the compiler about
 *   it.
 *    */
static inline void cpuid(int code, uint32_t *a, uint32_t *d)
{
	__asm volatile("cpuid":"=a"(*a), "=d"(*d):"a"(code):"ecx", "ebx");
}

/** issue a complete request, storing general registers output as a string
 *  */
static inline int cpuid_string(int code, uint32_t where[4])
{
	__asm volatile("cpuid":"=a"(*where), "=b"(*(where+1)),
	               "=c"(*(where + 2)), "=d"(*(where + 3)): "a"(code));
	return (int)where[0];
}

static inline bool cpu_msr_supported()
{
	uint32_t a, d; // eax, edx
	cpuid(1, &a, &d);
	return d & CPUID_FEAT_EDX_MSR;
}

static inline void cpuGetMSR(uint32_t msr, uint32_t *lo, uint32_t *hi)
{
	__asm volatile("rdmsr" : "=a"(*lo), "=d"(*hi) : "c"(msr));
}

static inline void cpuSetMSR(uint32_t msr, uint32_t lo, uint32_t hi)
{
	__asm volatile("wrmsr" : : "a"(lo), "d"(hi), "c"(msr));
}


// Write a byte out to the specified port.
static inline void outb(uint16_t port, uint8_t value)
{
	__asm volatile("outb %1, %0" : : "dN"(port), "a"(value));
}

static inline uint16_t inb(uint16_t port)
{
	uint8_t ret;
	__asm volatile("inb %1, %0" : "=a"(ret) : "dN"(port));
	return ret;
}

static inline void outw(uint16_t port, uint16_t value)
{
	__asm volatile("outw %1, %0" : : "dN"(port), "a"(value));
}

static inline uint16_t inw(uint16_t port)
{
	uint16_t ret;
	__asm volatile("inw %1, %0" : "=a"(ret) : "dN"(port));
	return ret;
}
static inline void enable_irq()
{
	__asm volatile("sti");
}

static inline void disable_irq()
{
	__asm volatile("cli");
}

static inline void cpu_relax()
{
	__asm volatile("hlt");
}

/* Compile read-write barrier */
#define wmb() asm volatile("": : :"memory")

/* Pause instruction to prevent excess processor bus usage */
#define cpu_relax() asm volatile("pause\n": : :"memory")

static inline unsigned xchg_32(void *ptr, unsigned x)
{
	__asm volatile("xchgl %0,%1"
				:"=r" ((unsigned) x)
				:"m" (*(volatile unsigned *)ptr), "0" (x)
				:"memory");

	return x;
}

static inline unsigned short xchg_16(void *ptr, unsigned short x)
{
	__asm volatile("xchgw %0,%1"
				:"=r" ((unsigned short) x)
				:"m" (*(volatile unsigned short *)ptr), "0" (x)
				:"memory");

	return x;
}

/* Test and set a bit */
static inline char atomic_bitsetandtest(void *ptr, int x)
{
	char out;
	__asm volatile("lock; bts %2,%1\n"
						"sbb %0,%0\n"
				:"=r" (out), "=m" (*(volatile long long *)ptr)
				:"Ir" (x)
				:"memory");

	return out;
}
#endif /* CPU_H_ */
