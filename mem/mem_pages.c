#include <mem/mem_pages.h>
#include <printk.h>
#include <OS_types.h>

#define PAGE_SIZE 4096

static uint32_t total_memory = 0;
static uintptr_t physical_start = 0;

void mem_init(multiboot_info_t *mbi)
{
	printk("Memory init:\r\ndetecting physical memory.\r\n");
	
	multiboot_memory_map_t* mmap = mbi->mmap_addr;
	int i = 0;
	
	while(mmap < mbi->mmap_addr + mbi->mmap_length) {
		mmap = (multiboot_memory_map_t*) ( (unsigned int)mmap + mmap->size + sizeof(unsigned int) );
		if (mmap->size == 0)
		{
			// That's the end of the list
			break;
		}

		printk("Memory region size: %u address: %llu length: %llu type: %u\r\n",
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

	printk("Physical memory zone set to: %u size: %u\r\n", physical_start, total_memory);


}

void mem_page_free(addr_t page)
{
}

addr_t mem_page_get()
{
	return NULL;
}
