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
#include <mem/memory.h>
#include <cpu.h>
#include <apic.h>
#include <printk.h>

#define IA32_APIC_BASE_MSR 0x1B
#define IA32_APIC_BASE_MSR_BSP 0x100 // Processor is a BSP
#define IA32_APIC_BASE_MSR_ENABLE 0x800
#define APIC_SPURIOUS_INTERRUPT_VECTOR      0x0F

/* APIC Timer definitions */
#define APIC_DIVIDE_CONFIG_REGISTER			0x3E0
#define APIC_INITIAL_COUNT_REGISTER			0x380
#define APIC_LVT_TIMER_REGISTER				0x320
#define APIC_CURRENT_COUNT_REGISTER			0x390

/* I8259 definitions, only for disabling it */
#define PIC1            0x20   /* IO base address for master PIC */
#define PIC2            0xA0   /* IO base address for slave PIC */
#define PIC1_COMMAND    PIC1
#define PIC1_DATA       (PIC1 + 1)
#define PIC2_COMMAND    PIC2
#define PIC2_DATA       (PIC2 + 1)



/** returns a 'true' value if the CPU supports APIC
 *  and if the local APIC hasn't been disabled in MSRs
 *  note that this requires CPUID to be supported.
 */
bool cpuHasAPIC()
{
	uint32_t eax, edx;
	cpuid(CPUID_GETFEATURES, &eax, &edx);
	return edx & CPUID_FEAT_EDX_APIC;
}

/* Set the physical address for local APIC registers */
void cpuSetAPICBase(uintptr_t apic)
{
	uint32_t edx = 0;
	uint32_t eax = (apic & 0xfffff100) | IA32_APIC_BASE_MSR_ENABLE;

	cpuSetMSR(IA32_APIC_BASE_MSR, eax, edx);
}

/**
 * Get the physical address of the APIC registers page
 * make sure you map it to virtual memory ;)
 */
uintptr_t cpuGetAPICBase()
{
	uint32_t eax, edx;
	cpuGetMSR(IA32_APIC_BASE_MSR, &eax, &edx);
	return (eax & 0xfffff000);
}
static uint32_t readAPICRegister(uint32_t reg)
{
	return *((volatile uint32_t *)(cpuGetAPICBase() + reg * 16));
}

static void writeAPICRegister(uint32_t reg, uint32_t value)
{
	*((volatile uint32_t *)(cpuGetAPICBase() + reg * 16)) = value;
}
static void disable_i8259()
{
	outb(0xff, PIC1_DATA);
	outb(0xff, PIC2_DATA);
}

void enableAPIC()
{

	/* First thing first, disable the PIC */

	disable_i8259();

	addr_t apic_base = cpuGetAPICBase();
	/* Hardware enable the Local APIC if it wasn't enabled */
	cpuSetAPICBase(apic_base);

	/* this usually maps to 0xFEE00000 */

	/* Idendity map the APIC base */
	mem_page_map(apic_base, apic_base, 0);

	printk("APIC Base was set to: 0x%x\r\n", apic_base);
	/* Set the Spourious Interrupt Vector Register bit 8 to start receiving interrupts */
	writeAPICRegister(APIC_SPURIOUS_INTERRUPT_VECTOR, readAPICRegister(APIC_SPURIOUS_INTERRUPT_VECTOR) | 0x100);



}



