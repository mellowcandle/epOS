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

#define DEBUG
#include <printk.h>
#undef DEBUG
static bool acpi_initalized = false;
static bool acpi_tables_initalized = false;

void acpi_early_init()
{

	FUNC_ENTER();
	ACPI_STATUS	rv;

	AcpiOsInitialize();

	rv = AcpiInitializeTables(NULL, 16, FALSE);

	if (ACPI_FAILURE(rv))
	{
		pr_fatal("ACPI table initalization failed\r\n");
		panic();
	}

	acpi_tables_initalized = true;

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
	printk("ACPI Error: %u\r\n", err_code);
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
	ACPI_TABLE_MADT * table = acpi_get_madt();
	assert(table != NULL);

	return table->Address;
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

