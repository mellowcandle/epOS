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

#include <acpi.h>
#include <mem/memory.h>
#include <kernel/bits.h>
#include <apic.h>
#define DEBUG
#include <printk.h>
#undef DEBUG
static bool acpi_initalized = false;
static bool acpi_tables_initalized = false;

typedef void (*acpi_subtable_handler)(ACPI_SUBTABLE_HEADER *subtable_header);

void acpi_madt_print_subtables();

void acpi_early_init()
{

	FUNC_ENTER();
	ACPI_STATUS	rv;

	AcpiOsInitialize();

	rv = AcpiInitializeTables(NULL, 16, FALSE);

	if (ACPI_FAILURE(rv))
	{
		pr_fatal("ACPI table initalization failed\r\r\n");
		panic();
	}

	acpi_tables_initalized = true;

	acpi_madt_print_subtables();
}

void acpi_init()
{
	FUNC_ENTER();

	ACPI_STATUS	rv;
	int 	err_code;

	rv = AcpiInitializeSubsystem();

	if (ACPI_FAILURE(rv))
	{
		err_code = 1;
		goto error;
	}

	rv = AcpiLoadTables();

	if (ACPI_FAILURE(rv))
	{
		err_code = 2;
		goto error;
	}

	rv = AcpiEnableSubsystem(ACPI_FULL_INITIALIZATION);

	if (ACPI_FAILURE(rv))
	{
		err_code = 3;
		goto error;
	}

	acpi_initalized = true;
	return;

error:
	AcpiTerminate();
	printk("ACPI Error: %u\r\r\n", err_code);
	panic();

}

ACPI_TABLE_FADT *acpi_get_fadt()
{
	FUNC_ENTER();
	ACPI_TABLE_HEADER *table;

	assert(acpi_tables_initalized);

	ACPI_STATUS status = AcpiGetTable(ACPI_SIG_FADT, 1, &table);

	if (ACPI_FAILURE(status))
	{
		return 0;
	}

	return (ACPI_TABLE_FADT *)table;
}

ACPI_TABLE_MADT *acpi_get_madt()
{
	FUNC_ENTER();
	ACPI_TABLE_HEADER *table;

	assert(acpi_tables_initalized);
	ACPI_STATUS status = AcpiGetTable(ACPI_SIG_MADT, 1, &table);

	if (ACPI_FAILURE(status))
	{
		return 0;
	}

	return (ACPI_TABLE_MADT *)table;
}

addr_t acpi_get_local_apic_addr()
{
	FUNC_ENTER();
	ACPI_TABLE_MADT *table = acpi_get_madt();
	assert(table != NULL);

	return table->Address;
}

bool acpi_8259_available()
{
	ACPI_TABLE_MADT *table = acpi_get_madt();
	return (table->Flags & BIT(0));
}

void acpi_shutdown()
{
	FUNC_ENTER();
	assert(acpi_initalized);

	AcpiEnterSleepStatePrep(5);
//	cli(); // disable interrupts
	AcpiEnterSleepState(5);
	panic(); // in case it didn't work!
}

void acpi_foreach_subtable(ACPI_TABLE_HEADER *table,
                           ACPI_SUBTABLE_HEADER *first,
                           acpi_subtable_handler handler)
{
	ACPI_SUBTABLE_HEADER *end = (ACPI_SUBTABLE_HEADER *)((uintptr_t)table + table->Length);
	ACPI_SUBTABLE_HEADER *subtable_header = first;

	while (subtable_header < end)
	{
		if (subtable_header->Length < sizeof(ACPI_SUBTABLE_HEADER))
		{
			break;
		}

		handler(subtable_header);

		subtable_header = (ACPI_SUBTABLE_HEADER *)((uintptr_t)subtable_header + subtable_header->Length);
	}
}

void acpi_madt_foreach_subtable(acpi_subtable_handler handler)
{
	ACPI_TABLE_MADT *madt = acpi_get_madt();
	assert(madt != NULL);

	acpi_foreach_subtable(&madt->Header, (ACPI_SUBTABLE_HEADER *)(madt + 1), handler);
}

void acpi_madt_configure_apic_subtable(ACPI_SUBTABLE_HEADER *header)
{
	ACPI_MADT_LOCAL_APIC *apic;
	ACPI_MADT_IO_APIC *ioapic;
	ACPI_MADT_INTERRUPT_OVERRIDE *int_override;
	ACPI_MADT_NMI_SOURCE *nmi_source;
	ACPI_MADT_LOCAL_APIC_NMI *lapic_nmi;
	ACPI_MADT_LOCAL_APIC_OVERRIDE *lapic_override;

	switch (header->Type)
	{
	case ACPI_MADT_TYPE_LOCAL_APIC:
		apic = (ACPI_MADT_LOCAL_APIC *)header;
		apic_configure_lapic(apic->Id, apic->ProcessorId, apic->LapicFlags);;
		break;

	case ACPI_MADT_TYPE_IO_APIC:
		ioapic = (ACPI_MADT_IO_APIC *)header;
		apic_configure_ioapic(ioapic->Id, ioapic->Address, ioapic->GlobalIrqBase);
		break;

	case ACPI_MADT_TYPE_INTERRUPT_OVERRIDE:
		int_override = (ACPI_MADT_INTERRUPT_OVERRIDE *)header;
		apic_configure_int_override(int_override->SourceIrq, int_override->GlobalIrq,
		                            int_override->IntiFlags);
		break;

	case ACPI_MADT_TYPE_NMI_SOURCE:
		nmi_source = (ACPI_MADT_NMI_SOURCE *)header;
		apic_configure_nmi_source(nmi_source->GlobalIrq, nmi_source->IntiFlags);
		break;

	case ACPI_MADT_TYPE_LOCAL_APIC_NMI:
		lapic_nmi = (ACPI_MADT_LOCAL_APIC_NMI *)header;
		apic_configure_lapic_nmi(lapic_nmi->ProcessorId, lapic_nmi->Lint,
		                         lapic_nmi->IntiFlags);
		break;

	case ACPI_MADT_TYPE_LOCAL_APIC_OVERRIDE:
		lapic_override = (ACPI_MADT_LOCAL_APIC_OVERRIDE *)header;
		apic_configure_lapic_override(lapic_override->Address);
		break;

	default:
		pr_error("Ooops. no handler found for type %u\r\n", header->Type);

	}
}

void acpi_configure_apic()
{
	acpi_madt_foreach_subtable(acpi_madt_configure_apic_subtable);
}

void acpi_print_subtable_header(ACPI_SUBTABLE_HEADER *header)
{
	printk("ACPI subtable Type=%d Length=%d\r\n", header->Type, header->Length);
}

void acpi_madt_print_subtable(ACPI_SUBTABLE_HEADER *header)
{
	ACPI_MADT_LOCAL_APIC *lapic;
	ACPI_MADT_IO_APIC *ioapic;
	ACPI_MADT_INTERRUPT_OVERRIDE *int_override;
	ACPI_MADT_NMI_SOURCE *nmi_source;
	ACPI_MADT_LOCAL_APIC_NMI *lapic_nmi;
	ACPI_MADT_LOCAL_APIC_OVERRIDE *lapic_override;

	switch (header->Type)
	{
	case ACPI_MADT_TYPE_LOCAL_APIC:
		lapic = (ACPI_MADT_LOCAL_APIC *)header;
		printk("LOCAL APIC Id=%d ProcessorId=%d LApicFlags=0x%x\r\n",
		       lapic->Id, lapic->ProcessorId, lapic->LapicFlags);
		break;

	case ACPI_MADT_TYPE_IO_APIC:
		ioapic = (ACPI_MADT_IO_APIC *)header;
		printk("I/O APIC Id=%d Address=0x%x GlobalIrqBase=0x%x\r\n",
		       ioapic->Id, ioapic->Address, ioapic->GlobalIrqBase);
		break;

	case ACPI_MADT_TYPE_INTERRUPT_OVERRIDE:
		int_override = (ACPI_MADT_INTERRUPT_OVERRIDE *)header;
		printk("Interrupt Override Bus=%d SourceIrq=0x%x GlobalIrq=0x%x IntiFlags=0x%x\r\n",
		       int_override->Bus, int_override->SourceIrq,
		       int_override->GlobalIrq, int_override->IntiFlags);
		break;

	case ACPI_MADT_TYPE_NMI_SOURCE:
		nmi_source = (ACPI_MADT_NMI_SOURCE *)header;
		printk("NMI Source IntiFlags=0x%x GlobalIrq=0x%x\r\n",
		       nmi_source->IntiFlags, nmi_source->GlobalIrq);
		break;

	case ACPI_MADT_TYPE_LOCAL_APIC_NMI:
		lapic_nmi = (ACPI_MADT_LOCAL_APIC_NMI *)header;
		printk("LApic NMI ProcessorId=%d IntiFlags=0x%x Lint=%d\r\n",
		       lapic_nmi->ProcessorId, lapic_nmi->IntiFlags, lapic_nmi->Lint);
		break;

	case ACPI_MADT_TYPE_LOCAL_APIC_OVERRIDE:
		lapic_override = (ACPI_MADT_LOCAL_APIC_OVERRIDE *)header;
		printk("LApic Override Address=0x%llx\r\n", lapic_override->Address);
		break;

	default:
		printk("Subtable type was unhandled\r\n");
	}
}

void acpi_madt_print_subtables()
{
	printk("ACPI MADT: Length=%d\r\n", acpi_get_madt()->Header.Length);
	acpi_madt_foreach_subtable(acpi_madt_print_subtable);
}



