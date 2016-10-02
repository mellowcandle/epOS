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

#include <types.h>
#include <cpu.h>
#include <kernel/bits.h>
#include <printk.h>
#include <acpi.h>
#include <isr.h>

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

static int kbd_8042_self_test();
static bool ps2_dual_channel = false;
static int ps2_port_test(bool dual);
void kbd_8042_enable(uint8_t port);
void kbd_8042_disable(uint8_t port);
void kbd_8042_poll();

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

static void kbd_8042_write_data(uint8_t cmd)
{
	outb(KBD_8042_DATA_PORT, cmd);
}

static uint8_t kbd_8042_read_config()
{
	kbd_8042_write_cmd(0x20);
	return kbd_8042_data();
}

static void kbd_8042_write_config(uint8_t config)
{
	kbd_8042_write_cmd(0x60);
	kbd_8042_write_data(config);
}

void kbd_irq_handler(registers_t regs)
{
	FUNC_ENTER();


	while ((kbd_8042_status() & STATUS_OUTPUT_BUF_STATUS))
	{
		pr_debug("read once\r\n");
		kbd_8042_data();
	}
}

void kbd_8042_init()
{
	uint8_t config;

	/* First, disable the device */
	kbd_8042_write_cmd(0xAD);
	kbd_8042_write_cmd(0xA7);

	/* Flush the buffer */
	while (kbd_8042_status() & STATUS_OUTPUT_BUF_STATUS)
	{
		(void) kbd_8042_data();
	}

	config = kbd_8042_read_config();
	pr_debug("KBD Initial Config was: %x\r\n", config);

	if (BIT_CHECK(config, 5))
	{
		ps2_dual_channel = true;
		pr_info("Detected dual channel PS2 controller\r\n");
	}

	/* Clean up the config */
	BIT_CLEAR(config, 0); // Disable first PS/2 port interrupt
	BIT_CLEAR(config, 1); // Disable second PS/2 port interrupt
	BIT_CLEAR(config, 6); // Disable first PS/2 port translation

	kbd_8042_write_config(config);
	pr_debug("KBD Config set to: %x\r\n", config);

	if (kbd_8042_self_test())
	{
		return;
	}

	if (ps2_port_test(ps2_dual_channel))
	{
		return;
	}

	register_interrupt_handler(IRQ1, &kbd_irq_handler);
	ioapic_map_irq(1, IRQ1);
	ioapic_irq_unmask(1);


	kbd_8042_enable(1);
//	kbd_8042_enable(2);
}

void kbd_8042_enable(uint8_t port)
{
	uint8_t config;
	config = kbd_8042_read_config();

	if (port == 1)
	{
		BIT_SET(config, 0);
	}
	else if (port == 2)
	{
		BIT_SET(config, 1);
	}
	else
	{
		pr_error("Unknown PS2 port\r\n");
		return;
	}

	kbd_8042_write_config(config);

	if (port == 1)
	{
		kbd_8042_write_cmd(0xAE);
	}
	else
	{
		kbd_8042_write_cmd(0xA8);
	}

}
void kbd_8042_disable(uint8_t port)
{
	if (port == 1)
	{
		kbd_8042_write_cmd(0xAD);
	}
	else if (port == 2)
	{
		kbd_8042_write_cmd(0xA7);
	}
	else
	{
		pr_error("Unknown PS2 port\r\n");
		return;
	}

}
int kbd_8042_self_test()
{
	uint8_t data;
	kbd_8042_write_cmd(0xAA);

	while (!(kbd_8042_status() & STATUS_OUTPUT_BUF_STATUS))
		;

	data = kbd_8042_data();

	if (data != 0x55)
	{
		pr_error("8042 kbd self test failed\r\n");
		return -1;
	}

	return 0;
}
int ps2_port_test(bool dual)
{
	uint8_t data;
	kbd_8042_write_cmd(0xAB);

	while (!(kbd_8042_status() & STATUS_OUTPUT_BUF_STATUS))
		;

	data = kbd_8042_data();

	if (data)
	{
		pr_error("PS2 port 1 test failed\r\n");
		return -1;
	}

	if (!dual)
	{
		return 0;
	}

	kbd_8042_write_cmd(0xA9);

	while (!(kbd_8042_status() & STATUS_OUTPUT_BUF_STATUS))
		;

	data = kbd_8042_data();

	if (data)
	{
		pr_error("PS2 port 2 test failed\r\n");
		return -1;
	}

	return 0;
}

void kbd_8042_poll()
{
	uint8_t data;

	while (1)
	{

		while (!(kbd_8042_status() & STATUS_OUTPUT_BUF_STATUS))
			;

		data = kbd_8042_data();
		printk("Data read from controller: 0x%x\r\n", data);
	}
}

bool kbd_8042_avail()
{
	ACPI_TABLE_FADT *fadt = acpi_get_fadt();

	if (fadt->Header.Revision < 2)
	{
		pr_debug("ACPI FADT version < 2 - Assuming 8042 availability\r\n");
		return true;
	}

	return (fadt->BootFlags & ACPI_FADT_8042);
}

