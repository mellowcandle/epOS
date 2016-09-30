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

#include <types.h>
#include <cpu.h>
#include <kernel/bits.h>
#include <printk.h>
#include <acpi.h>

/* Port addresses */
#define KBD_8042_DATA_PORT	0x60
#define KBD_8042_CMD_PORT	0x64

/* Status register bits */
#define STATUS_OUTPUT_BUF_STATUS	BIT(0)
#define STATUS_INPUT_BUF_STATUS		BIT(1)
#define STATUS_SYSTEM_FLAG			BIT(2)
#define STATUS_CMD_DATA				BIT(3)
#define STATUS_INHIBIT_SWITCH		BIT(4)
#define STATUS_TRANSMIT_TIMEOUT		BIT(5)
#define STATUS_RECIEVE_TIMEOUT		BIT(6)
#define STATUS_PARITY_ERROR			BIT(7)


ACPI_TABLE_FADT *acpi_get_fadt();

static uint8_t kbd_8042_status()
{
	return inb(KBD_8042_CMD_PORT);
}

static uint8_t kbd_8042_data()
{
	return inb(KBD_8042_DATA_PORT);
}

static void kbd_8042_write_cmd(uint8_t cmd)
{
	outb(KBD_8042_CMD_PORT, cmd);
}

static uint8_t kbd_8042_read_config()
{
	kbd_8042_write_cmd(0x20);
	return kbd_8042_data();
}

void kbd_8042_init()
{
	/* First, disable the device */
	kbd_8042_write_cmd(0xAD);
	kbd_8042_write_cmd(0xA7);

	/* Flush the buffer */
	while (kbd_8042_status() & STATUS_OUTPUT_BUF_STATUS)
	{
		(void) kbd_8042_data();
	}

}

bool kbd_8042_avail()
{
	ACPI_TABLE_FADT * fadt = acpi_get_fadt();

	if (fadt->Header.Revision < 2)
	{
		pr_info("ACPI FADT version < 2 - Assuming 8042 availability\r\n");
		return true;
	}

	return (fadt->BootFlags & ACPI_FADT_8042);
}

