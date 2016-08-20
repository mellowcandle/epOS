#ifndef MEM_PAGES_INCLUDE
#define MEM_PAGES_INCLUDE

#include <OS_types.h>
#include <boot/multiboot.h>

typedef uintptr_t addr_t;

void mem_init(multiboot_info_t *mbi);
void mem_page_free(addr_t page);
addr_t mem_page_get(void);

#endif
