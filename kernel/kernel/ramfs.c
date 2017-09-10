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
//#define DEBUG

#include <tar.h>
#include <lib/list.h>
#include <kmalloc.h>
#include <printk.h>
#include <lib/string.h>

#define TAR_BLOCK_SIZE 512
#define BLOCK_MASK (~(TAR_BLOCK_SIZE-1))
#define BLOCK_ALIGN_UP(_x) (((_x)+TAR_BLOCK_SIZE - 1) & BLOCK_MASK)


static LIST(ramfs_headers);
typedef struct
{
	tar_header_t *ptr;
	list_t list_head;
} ramfs_node;

uint32_t ramfs_atoi(const char *in, uint32_t len)
{

	FUNC_ENTER();
	uint32_t size = 0;
	uint32_t count = 1;

	for (; len > 0; len--, count *= 8)
	{
		size += ((in[len - 1] - '0') * count);
	}

	return size;

}
int ramfs_find_node(const char *name, tar_header_t **ptr)
{
	ramfs_node *node;
	list_for_each_entry(node, &ramfs_headers, list_head)
	{
		if (strncmp(node->ptr->header.name, name, strlen(name)) == 0)
		{
			pr_debug("Found the node\r\n");
			*ptr = node->ptr;
			return 0;
		}
	}
	pr_error("Didn't find the node\r\n");
	return -1;
}

int parse_ramfs(void *ramfs, uint32_t size)
{
	FUNC_ENTER();
	uint32_t offset = 0;
	uint32_t adapt;
	ramfs_node *new_header;
	uint8_t *pos;

	while (size)
	{
		pos = (uint8_t *) ramfs + offset;

		if (pos[0] == 0) // Detect EOF
		{
			break;
		}

		new_header = kmalloc(sizeof(ramfs_node));
		assert(new_header);
		new_header->ptr = (tar_header_t *)pos;

		adapt = TAR_BLOCK_SIZE +
		        BLOCK_ALIGN_UP(ramfs_atoi(new_header->ptr->header.size, 11));
		size -= adapt;
		offset += adapt;

		list_add_tail(&new_header->list_head, &ramfs_headers);
		pr_debug("RAMFS Entry added: %c %s\r\n", new_header->ptr->header.linkflag,
		         new_header->ptr->header.name);
	}

	return 0;
}

int init_ramfs(addr_t phy_addr, uint32_t len)
{
	FUNC_ENTER();
	pr_debug("phy_addr: 0x%x len: 0x%x\r\n", phy_addr, len);
	uint32_t pagecnt = divide_up(len, PAGE_SIZE);
	addr_t aligned_addr = PAGE_ALIGN_DOWN(phy_addr);

	void *ramfs = mem_page_map_kernel(aligned_addr, pagecnt, READ_WRITE_KERNEL);

	if (!ramfs)
	{
		pr_fatal("Can't map memory\r\n");
		panic();
	}

	pr_info("RAMFS: 0x%x->0x%x\r\n", phy_addr, (addr_t) ramfs);

	ramfs += (phy_addr - aligned_addr);
	return parse_ramfs(ramfs, len);
}
