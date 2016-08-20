#include <mem/mem_pages.h>
#include <printk.h>
void mem_init(multiboot_info_t *mbi)
{
	printk("Memory init:\r\ndetecting physical memory.\r\n");
	
	multiboot_memory_map_t* mmap = mbi->mmap_addr;
	int i = 0;
	
	while(mmap < mbi->mmap_addr + mbi->mmap_length) {
		mmap = (multiboot_memory_map_t*) ( (unsigned int)mmap + mmap->size + sizeof(unsigned int) );
		printk("Memory region size: %u address: %llu length: %llu type: %u\r\n",
			mmap->size, mmap->addr, mmap->len, mmap->type);
	}

}

void mem_page_free(addr_t page)
{
}

addr_t mem_page_get()
{
	return NULL;
}
