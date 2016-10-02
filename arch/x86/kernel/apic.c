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
#define DEBUG

#include <types.h>
#include <kernel/bits.h>
#include <mem/memory.h>
#include <cpu.h>
#include <apic.h>
#include <printk.h>
#include <acpi.h>
#include <lib/list.h>
#include <lib/kmalloc.h>

#define IA32_APIC_BASE_MSR 0x1B
#define IA32_APIC_BASE_MSR_BSP 0x100 // Processor is a BSP
#define IA32_APIC_BASE_MSR_ENABLE 0x800

#define APIC_LOCAL_ID_REGISTER				0x20
#define APIC_TASK_PRIORITY_REGISTER			0x80
#define APIC_SPURIOUS_INTERRUPT_VECTOR      0xF0
#define APIC_EOI_REGISTER					0xB0
#define APIC_PERFORMANCE_MONITOR_COUNTER_REGISTER 0x340
#define APIC_LVT_LINT0						0x350
#define APIC_LVT_LINT1						0x360
#define APIC_LVT_ERROR_REGISTER				0x370
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


typedef struct
{
	uint8_t id;
	addr_t p_addr;
	void *v_addr;
} lapic_t;


static lapic_t lapic;
LIST(ioapic_l);



ioapic_t *irq_to_ioapic(uint8_t irq)
{
	//TODO: implement it for real
	//
	ioapic_t *entry;
	entry = list_first_entry(&ioapic_l, ioapic_t, head);

	return entry;
}
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
	return *((volatile uint32_t *)(cpuGetAPICBase() + reg));
}

static void writeAPICRegister(uint32_t reg, uint32_t value)
{
	*((volatile uint32_t *)(cpuGetAPICBase() + reg)) = value;
}
static void disable_i8259()
{
	FUNC_ENTER();
	outb(0x20, PIC1_COMMAND);
	outb(0x20, PIC2_COMMAND);
	outb(0xff, PIC1_DATA);
	outb(0xff, PIC2_DATA);
}


void apic_configure_lapic(uint8_t id, uint8_t processor_id, uint16_t flags)
{
	FUNC_ENTER();

	static int visited = 0;

	if (visited)
	{
		return;
	}

	visited = 1;

	//TODO: do we need to keep the data we get ??

	/* this usually maps to 0xFEE00000 */
	lapic.p_addr = acpi_get_local_apic_addr();

	/* Hardware enable the Local APIC if it wasn't enabled */
	cpuSetAPICBase(lapic.p_addr);

	/* Idendity map the APIC base */

	lapic.v_addr = (void *) lapic.p_addr;
	mem_page_map(lapic.p_addr, (addr_t) lapic.v_addr, 0);

	pr_info("APIC Base mapping 0x%x -> 0x%x\r\n", lapic.p_addr, (addr_t) lapic.v_addr);

	/* First thing first, disable the PIC */
	if (flags && BIT(0))
	{
		pr_info("Disabling the i8259 IRQ chip\r\n");
		disable_i8259();
	}

	/* Inhibit soft init delivery */
//	writeAPICRegister(APIC_TASK_PRIORITY_REGISTER, 0x20);
	/* Disable timer interrupts */
	writeAPICRegister(APIC_LVT_TIMER_REGISTER, 0x10000);

	/* disable performance counter interrupts */
	writeAPICRegister(APIC_PERFORMANCE_MONITOR_COUNTER_REGISTER, 0x10000);

	/* enable normal external interrupts */
	writeAPICRegister(APIC_LVT_LINT0, 0x08700);

	/* enable normal NMI processing */
	writeAPICRegister(APIC_LVT_LINT1, 0x400);

	/* disable error interrupts */
	/* writeAPICRegister(APIC_LVT_ERROR_REGISTER, 0x10000); */

	/* Set the Spourious Interrupt Vector Register bit 8 to start receiving interrupts */
	writeAPICRegister(APIC_SPURIOUS_INTERRUPT_VECTOR, readAPICRegister(APIC_SPURIOUS_INTERRUPT_VECTOR) | 0x100);
}

void apic_configure_ioapic(uint8_t id, addr_t address, addr_t irq_base)
{

	FUNC_ENTER();

	ioapic_t *ioapic = kmalloc(sizeof(ioapic_t));
	assert(ioapic);

	ioapic->id = id;
	ioapic->p_addr = address;
	ioapic->v_addr = (void *) address; // Identity map
	ioapic->global_irq_base = irq_base;

	mem_page_map(ioapic->p_addr, (addr_t) ioapic->v_addr, 0);

	pr_info("IOAPIC Base mapping 0x%x -> 0x%x\r\n", ioapic->p_addr, (addr_t) ioapic->v_addr);
	pr_info("IOAPIC IRQ Base: %u\r\n", ioapic->global_irq_base);
	list_add(&ioapic->head, &ioapic_l);

	ioapic_santize(ioapic);
}

void apic_eoi()
{
	writeAPICRegister(APIC_EOI_REGISTER, 0);
}

void apic_configure_int_override(uint8_t irq_src, uint32_t global_irq, uint16_t flags)
{
	FUNC_ENTER();
	pr_info("APIC: IRQ override %u -> %u : 0x%x\r\n", irq_src, global_irq, flags);
}

void apic_configure_nmi_source(uint32_t global_irq, uint16_t flags)
{
	FUNC_ENTER();
}
void apic_configure_lapic_nmi(uint8_t cpu_id, uint16_t flags, uint8_t lint)
{
	FUNC_ENTER();
}
void apic_configure_lapic_override(uint64_t address)
{
	FUNC_ENTER();
}


