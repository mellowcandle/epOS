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

#ifndef FILE_H_HMSG2AFB
#define FILE_H_HMSG2AFB

#include <kernel.h>

struct file;
struct file_ops;
struct inode;

extern const struct file_ops ramfs_fops;
extern const struct file_ops devfs_fops;

struct file_ops {
	off_t (*llseek) (struct file *, off_t, int);
	size_t (*read) (struct file *, char *, size_t, off_t *);
	size_t (*write) (struct file *, const char *, size_t, off_t *);
	long (*ioctl) (struct file *, unsigned int, unsigned long);
	int (*open) (struct inode *, struct file *);
	int (*flush) (struct file *);
	int (*release) (struct inode *, struct file *);

};

struct file {
	int fd;
	const struct file_ops *f_ops;
	uint32_t size;
	off_t offset;
	void *addr;
	void *priv;
};

struct inode {
};

int get_next_fd(struct file *filp);
struct file * get_file_from_fd(int fd);
int set_fd(struct file *filp, int fd);

#endif /* end of include guard: FILE_H_HMSG2AFB */
