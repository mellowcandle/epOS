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

#include <vfs.h>

uint32_t read_fs(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer)
{
	// Has the node got a read callback?
	if (node->read != 0)
		return node->read(node, offset, size, buffer);
	else
		return 0;
}

uint32_t write_fs(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer)
{
	// Has the node got a write callback?
	if (node->write != 0)
		return node->write(node, offset, size, buffer);
	else
		return 0;
}

void open_fs(fs_node_t *node, uint8_t read, uint8_t write)
{
	// Has the node got an open callback?
	if (node->open != 0)
		return node->open(node);
}

void close_fs(fs_node_t *node)
{
	// Has the node got a close callback?
	if (node->close != 0)
		return node->close(node);
}

struct dirent *readdir_fs(fs_node_t *node, uint32_t index)
{
	// Is the node a directory, and does it have a callback?
	if ( (node->flags&0x7) == FS_DIRECTORY &&
			node->readdir != 0 )
		return node->readdir(node, index);
	else
		return 0;
}

fs_node_t *finddir_fs(fs_node_t *node, char *name)
{
	// Is the node a directory, and does it have a callback?
	if ( (node->flags&0x7) == FS_DIRECTORY &&
			node->finddir != 0 )
		return node->finddir(node, name);
	else
		return 0;
}
