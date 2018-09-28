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

#include <lib/circular.h>
#include <kmalloc.h>
#include <kernel.h>
#include <printk.h>
#include <bits.h>

static size_t _circ_buffer_space_left(circ_buffer_t *buffer);

circ_buffer_t *create_circ_buffer(size_t size)
{
	circ_buffer_t *tmp;

	if (!is_power_of_2(size)) {
		pr_error("Circular buffer size must be a power of 2");
		return NULL;
	}

	tmp = kmalloc(sizeof(circ_buffer_t));

	if (!tmp) {
		pr_error("circular buffer allocation failed\r\n");
		return NULL;
	}

	tmp->buffer = kmalloc(size);

	if (!tmp->buffer) {
		pr_error("circular buffer allocation failed\r\n");
		kfree(tmp);
		return NULL;
	}

	tmp->size = size;
	tmp->read_idx = 0;
	tmp->write_idx = 0;

	return tmp;
}
int read_circ_buffer(circ_buffer_t *buffer, size_t max_len, void *to)
{
	size_t actual_read = buffer->size - _circ_buffer_space_left(buffer);
	actual_read = MIN(max_len, actual_read);

	for (size_t i = 0; i < actual_read; i++) {
		((char *)to)[i] = buffer->buffer[buffer->read_idx % buffer->size];
		buffer->read_idx = (buffer->read_idx + 1) % buffer->size;
	}

	return actual_read;
}

int write_circ_buffer(circ_buffer_t *buffer, size_t max_len, void *from)
{
	size_t actual_write = MIN(max_len, _circ_buffer_space_left(buffer));

	for (size_t i = 0; i < actual_write; i++) {
		buffer->buffer[buffer->write_idx % buffer->size] = ((char *)from)[i];
		buffer->write_idx = (buffer->write_idx + 1) % buffer->size;
	}

	return actual_write;
}

bool circ_buffer_is_empty(circ_buffer_t *buffer)
{
	if (!buffer) {
		pr_error("buffer parameter is null");
		panic();
	}

	return (buffer->read_idx == buffer->write_idx);
}

bool circ_buffer_is_full(circ_buffer_t *buffer)
{
	if (!buffer) {
		pr_error("buffer parameter is null");
		panic();
	}

	return (buffer->read_idx == (buffer->write_idx + 1));
}

static size_t _circ_buffer_space_left(circ_buffer_t *buffer)
{

	if (buffer->read_idx == buffer->write_idx)
		return buffer->size - 1;
	else if (buffer->read_idx > buffer->write_idx)
		return (buffer->read_idx - buffer->write_idx);
	else
		return (buffer->size - 1 - (buffer->write_idx - buffer->read_idx));
}

void destroy_circ_buffer(circ_buffer_t *buf)
{
	if (!buf) {
		pr_error("buffer parameter is null");
		panic();
	}

	kfree(buf->buffer);
	kfree(buf);

}
