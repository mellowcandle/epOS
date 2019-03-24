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
#define DEBUG

#include <lib/list.h>
#include <kmalloc.h>
#include <printk.h>
#include <lib/string.h>
#include <file.h>

static LIST(devfs_drivers);

static off_t devfs_llseek(struct file *filp, off_t off, int flags)
{
	FUNC_ENTER();
	return 0;
}

static size_t devfs_read(struct file * filp, char *buf, size_t size, off_t *offset)
{
	FUNC_ENTER();
	return 0;
}

static long devfs_ioctl(struct file *filp, unsigned int ctl, unsigned long arg)
{
	FUNC_ENTER();
	return 0;
}

static int devfs_open(struct inode *inode, struct file *filp)
{

	FUNC_ENTER();
	return 0;
}

static int devfs_release(struct inode *inode, struct file *filp)
{
	FUNC_ENTER();
	return 0;
}
static size_t devfs_write(struct file *filp, const char *buf, size_t size, off_t *offset)
{
	FUNC_ENTER();
	return 0;
}

static int devfs_flush(struct file *filp)
{
	FUNC_ENTER();
	return 0;
}

int devfs_bind(const char *path, struct file *filp)
{
	struct char_driver *driver;

	filp->priv = NULL;
	list_for_each_entry(driver, &devfs_drivers, list)
		if(!strcmp(path, driver->node_name)) {
			filp->priv = driver;
			pr_debug("Matched %s for bind\n",
				 driver->node_name);
			break;
		}

	if (filp->priv)
		return 0;
	else
		return -ENOENT;

}

const struct file_ops devfs_fops = {
	.llseek = devfs_llseek,
	.read = devfs_read,
	.write = devfs_write,
	.flush = devfs_flush,
	.ioctl = devfs_ioctl,
	.open = devfs_open,
	.release = devfs_release,
};

int devfs_register(struct char_driver *drv)
{
	pr_debug("Registering %s to devfs\n", drv->name);
	list_add_tail(&drv->list, &devfs_drivers);

	return 0;
}
