#include <mem/mem_pages.h>
#include <printk.h>
#include <OS_types.h>

#define PAGE_SIZE 4096

// These are defined in the linker script, with these we calculate the size of the kernel on runtime.
extern unsigned long kernel_start;
extern unsigned long kernel_end;

static uint32_t total_memory = 0;
static uintptr_t physical_start = 0;
static uintptr_t pages_start;

static uint32_t total_pages;
static uint32_t *page_bitmap = NULL;

void mem_init(multiboot_info_t *mbi)
{
	
	uint32_t reserved_pages;
	uint32_t kernel_size;
	uint32_t required_kernel_pages;

	printk("Memory init:\r\ndetecting physical memory.\r\n");
	
	multiboot_memory_map_t* mmap = mbi->mmap_addr;
	
	while(mmap < mbi->mmap_addr + mbi->mmap_length) {
		mmap = (multiboot_memory_map_t*) ( (unsigned int)mmap + mmap->size + sizeof(unsigned int) );
		if (mmap->size == 0)
		{
			// That's the end of the list
			break;
		}

		printk("Memory region size: %u address: 0x%llx length: 0x%llx type: %u\r\n",
			mmap->size, mmap->addr, mmap->len, mmap->type);
		// We only support one zone, we'll take the biggest.
		//
		if (mmap->type == 1)
		{
			if (mmap->len > total_memory)
			{
				total_memory = mmap->len;
				physical_start = mmap->addr;
			}
		}

	}

	if (total_memory == 0)
	{
		printk("No physical memory!\r\n");
		panic();
	}

	kernel_size = (uint32_t) (&kernel_end - &kernel_start);
	required_kernel_pages = (kernel_size / PAGE_SIZE) + 1;
	
	printk("Kernel start: 0x%x, kernel end: 0x%x\r\n", &kernel_start, &kernel_end);
	printk("Kernel occupies 0x%x bytes, consuming %u pages\r\n", kernel_size, required_kernel_pages);

	physical_start += required_kernel_pages * PAGE_SIZE;
	total_memory -= required_kernel_pages * PAGE_SIZE;
	total_pages = total_memory / PAGE_SIZE;

	printk("Physical memory zone set to: 0x%x size: 0x%x\r\n", physical_start, total_memory);
	printk("Number of pages: %u\r\n", total_pages);

	// Now we calculate how many pages we need to reserve to ourselves for physical allocator management.
	reserved_pages = (total_pages / 32 * sizeof(uint32_t)) / PAGE_SIZE;
	if ((total_pages / 32 * sizeof(uint32_t)) % PAGE_SIZE)
		reserved_pages++;

	printk("reserved_pages = %u\r\n", reserved_pages);

	page_bitmap = physical_start;
	pages_start = page_bitmap + (reserved_pages * PAGE_SIZE);

	// Clear the memory bitmap
	memset(page_bitmap, 0, reserved_pages * PAGE_SIZE);

}

void mem_page_free(addr_t page)
{
}

addr_t mem_page_get()
{
	return NULL;
}
