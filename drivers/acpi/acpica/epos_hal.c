
//#define DEBUG

#include <acpica/acpi.h>
#include <lib/kmalloc.h>
#include <printk.h>
#include <mem/memory.h>
#include <kernel/bits.h>
#include <cpu.h>
static heap_t acpi_heap;

ACPI_STATUS AcpiOsInitialize()
{
	FUNC_ENTER();
	static bool init_done = false;

	if (!init_done)
	{
		mem_heap_init(&acpi_heap, 0xE0000000, 0x1000000);
		init_done = true;
	}

	return 0;
}

ACPI_STATUS AcpiOsTerminate()
{
	FUNC_ENTER();
	mem_heap_destroy(&acpi_heap);
	return 0;
}


ACPI_PHYSICAL_ADDRESS AcpiOsGetRootPointer()
{
	ACPI_PHYSICAL_ADDRESS  Ret;
	FUNC_ENTER();
	Ret = 0;
	AcpiFindRootPointer(&Ret);
	return Ret;
}

ACPI_STATUS AcpiOsPredefinedOverride(const ACPI_PREDEFINED_NAMES *PredefinedObject, ACPI_STRING *NewValue)
{
	FUNC_ENTER();
	*NewValue = NULL;
	return 0;
}


ACPI_STATUS AcpiOsTableOverride(ACPI_TABLE_HEADER *ExistingTable, ACPI_TABLE_HEADER **NewTable)
{
	FUNC_ENTER();
	*NewTable = NULL;
	return 0;
}
ACPI_STATUS AcpiOsPhysicalTableOverride(ACPI_TABLE_HEADER *ExistingTable, ACPI_PHYSICAL_ADDRESS *NewAddress, UINT32 *NewTableLength)
{
	FUNC_ENTER();
	*NewAddress = 0;
	return 0;
}

void *AcpiOsMapMemory(ACPI_PHYSICAL_ADDRESS PhysicalAddress, ACPI_SIZE Length)
{
	FUNC_ENTER();
	char *ret_addr;
	uint32_t pages = divide_up(Length, PAGE_SIZE);
	ACPI_PHYSICAL_ADDRESS aligned_addr = PAGE_ALIGN_DOWN(PhysicalAddress);

	ret_addr = mem_heap_map(&acpi_heap, pages, aligned_addr);
	assert(ret_addr);
	ret_addr += (PhysicalAddress - aligned_addr);

	return ret_addr;

}

void AcpiOsUnmapMemory(void *where, ACPI_SIZE length)
{
	FUNC_ENTER();
}

ACPI_STATUS AcpiOsGetPhysicalAddress(
    void                    *LogicalAddress,
    ACPI_PHYSICAL_ADDRESS   *PhysicalAddress)
{
	FUNC_ENTER();
	*PhysicalAddress = 	virt_to_phys((addr_t) LogicalAddress);
	return 0;
}

void *AcpiOsAllocate(ACPI_SIZE Size)
{
	FUNC_ENTER();
	return kmalloc(Size);
}

void AcpiOsFree(void *Memory)
{
	FUNC_ENTER();
	kfree(Memory);
}

BOOLEAN AcpiOsReadable(void *Memory, ACPI_SIZE Length)
{
	FUNC_ENTER();
	return true;
}

BOOLEAN AcpiOsWritable(void *Memory, ACPI_SIZE Length)
{
	FUNC_ENTER();
	return true;
}

void AcpiOsVprintf(const char *Format, va_list Args)
{
	FUNC_ENTER();
	vprintk(Format, Args);
}

ACPI_THREAD_ID AcpiOsGetThreadId()
{
	FUNC_ENTER();
	return 1;
}

ACPI_STATUS AcpiOsExecute(ACPI_EXECUTE_TYPE Type, ACPI_OSD_EXEC_CALLBACK Function, void *Context)
{
	FUNC_ENTER();
}

void AcpiOsSleep(UINT64 Milliseconds)
{
	FUNC_ENTER();
}

void AcpiOsStall(UINT32 Microseconds)
{
	FUNC_ENTER();
}


ACPI_STATUS AcpiOsCreateSemaphore(UINT32 MaxUnits, UINT32 InitialUnits, ACPI_SEMAPHORE *OutHandle)
{
	FUNC_ENTER();
	return 0;
}
ACPI_STATUS AcpiOsDeleteSemaphore(ACPI_SEMAPHORE Handle)
{
	FUNC_ENTER();
	return 0;
}

ACPI_STATUS AcpiOsWaitSemaphore(ACPI_SEMAPHORE Handle, UINT32 Units, UINT16 Timeout)
{
	FUNC_ENTER();
	return 0;
}

ACPI_STATUS AcpiOsSignalSemaphore(ACPI_SEMAPHORE Handle, UINT32 Units)
{
	FUNC_ENTER();
	return 0;
}

ACPI_STATUS AcpiOsCreateLock(ACPI_SPINLOCK *OutHandle)
{
	FUNC_ENTER();
	return 0;
}

void AcpiOsDeleteLock(ACPI_HANDLE Handle)
{
	FUNC_ENTER();
}

ACPI_CPU_FLAGS AcpiOsAcquireLock(ACPI_SPINLOCK Handle)
{
	FUNC_ENTER();
	return 0;
}

void AcpiOsReleaseLock(ACPI_SPINLOCK Handle, ACPI_CPU_FLAGS Flags)
{
	FUNC_ENTER();
}
ACPI_STATUS AcpiOsInstallInterruptHandler(UINT32 InterruptLevel, ACPI_OSD_HANDLER Handler, void *Context)
{
	FUNC_ENTER();
	return 0;
}

ACPI_STATUS AcpiOsRemoveInterruptHandler(UINT32 InterruptNumber, ACPI_OSD_HANDLER Handler)
{
	return 0;
}

void AcpiOsWaitEventsComplete(void)
{
	FUNC_ENTER();
}
void ACPI_INTERNAL_VAR_XFACE
AcpiOsPrintf(const char *Format, ...)
{
	va_list arg;
	int done;
	FUNC_ENTER();

	va_start(arg, Format);
	done = vprintk(Format, arg);
	va_end(arg);
	printk("\r");

}


ACPI_STATUS
AcpiOsReadPort(ACPI_IO_ADDRESS Address, UINT32 *Value, UINT32 Width)
{
	FUNC_ENTER();

	switch (Width)
	{
	case 8:
		*Value = inb(Address);
		break;

	case 16:
		*Value = inw(Address);
		break;

	//	case 32:	*Value = ind(Address);	break;
	default:
		printk("Bad parameter\r\n");
		return AE_BAD_PARAMETER;
	}

	return AE_OK;
}

ACPI_STATUS
AcpiOsWritePort(ACPI_IO_ADDRESS Address, UINT32 Value, UINT32 Width)
{
	FUNC_ENTER();

	switch (Width)
	{
	case 8:
		outb(Address, Value);
		break;

	case 16:
		outw(Address, Value);
		break;

	//	case 32:	outd(Address, Value);	break;
	default:
		printk("Bad parameter\r\n");
		return AE_BAD_PARAMETER;
	}

	return AE_OK;
}

ACPI_STATUS
AcpiOsReadMemory(
    ACPI_PHYSICAL_ADDRESS   Address,
    UINT64                  *Value,
    UINT32                  Width)
{
	FUNC_ENTER();
}

ACPI_STATUS AcpiOsWriteMemory(ACPI_PHYSICAL_ADDRESS   Address,    UINT64                  Value,    UINT32                  Width)
{
	FUNC_ENTER();
}


ACPI_STATUS AcpiOsReadPciConfiguration(ACPI_PCI_ID             *PciId,    UINT32                  Reg,    UINT64                  *Value,    UINT32                  Width)
{
	FUNC_ENTER();
}

ACPI_STATUS AcpiOsWritePciConfiguration(ACPI_PCI_ID             *PciId,    UINT32                  Reg,    UINT64                  Value,    UINT32                  Width)
{
	FUNC_ENTER();
}


UINT64 AcpiOsGetTimer(void)
{
	FUNC_ENTER();
}

ACPI_STATUS AcpiOsSignal(UINT32                  Function,    void                    *Info)
{
	FUNC_ENTER();
}

