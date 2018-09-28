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

#include <types.h>
#include <cpu.h>
#include <bits.h>
#include <printk.h>
#include <acpica/acpi.h>
#include <isr.h>
#include <lib/circular.h>

#define KBD_BUFFER_SIZE 128 // Must be power of 2
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

/* Keyboard state keys */
#define CAPS_LOCK_BIT 0
#define SCROLL_LOCK_BIT 1
#define NUM_LOCK_BIT 2

#define state_CAPS_LOCK		BIT(CAPS_LOCK_BIT)
#define state_SCROLL_LOCK	BIT(SCROLL_LOCK_BIT)
#define state_NUM_LOCK		BIT(NUM_LOCK_BIT)
#define state_LEFT_CTRL		BIT(3)
#define state_RIGHT_CTRL	BIT(4)
#define state_LEFT_ALT		BIT(5)
#define state_RIGHT_ALT		BIT(6)
#define state_LEFT_SHIFT	BIT(7)
#define state_RIGHT_SHIFT	BIT(8)

typedef struct {
	int state;
	circ_buffer_t *queue;
	bool ps2_dual_channel;
} kbd_device_t;

static kbd_device_t kbd;

ACPI_TABLE_FADT *acpi_get_fadt();

static uint8_t kbd_scan_table[128] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x00 - 0x0F
	0, 0, 0, 0, 0, 'q', '1', 0, 0, 0, 'z', 's', 'a', 'w', '2', 0, // 0x10 - 0x1F
	0, 'c', 'x', 'd', 'e', '4', '3', 0, 0, ' ', 'v', 'f', 't', 'r', '5', 0, // 0x20 - 0x2F
	0, 'n', 'b', 'h', 'g', 'y', '6', 0, 0, 0, 'm', 'j', 'u', '7', '8', 0,
	0, ',', 'k', 'i', 'o', '0', '9', 0, 0, '.', '/', 'l', ';', 'p', '-', 0,
	0, 0, '\'', 0, '[', '=', 0, 0, 0, 0, 0, ']', 0, '\\', 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
static uint8_t kbd_scan_table_shift[128] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0x00 - 0x0F
	0, 0, 0, 0, 0, 'q', '!', 0, 0, 0, 'z', 's', 'a', 'w', '@', 0, // 0x10 - 0x1F
	0, 'c', 'x', 'd', 'e', '$', '#', 0, 0, ' ', 'v', 'f', 't', 'r', '%', 0, // 0x20 - 0x2F
	0, 'n', 'b', 'h', 'g', 'y', '^', 0, 0, 0, 'm', 'j', 'u', '&', '*', 0,
	0, '<', 'k', 'i', 'o', ')', '(', 0, 0, '>', '?', 'l', ':', 'p', '_', 0,
	0, 0, '"', 0, '{', '+', 0, 0, 0, 0, 0, '}', 0, '|', 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static int kbd_8042_self_test();
static int ps2_port_test(bool dual);

static void kbd_8042_enable(uint8_t port);
static void kbd_8042_disable(uint8_t port);
static void kbd_8042_poll();
static void state_machine(uint8_t scan_code);

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

void kbd_irq_handler(registers_t *regs)
{

	while ((kbd_8042_status() & STATUS_OUTPUT_BUF_STATUS))
		state_machine(kbd_8042_data());
}

#define STATE_IDLE 0
#define STATE_PRE_RELEASE BIT(1)
#define STATE_PRE_RIGHT BIT(2)

static void state_machine(uint8_t scan_code)
{
	static int statemachine = STATE_IDLE;
	const uint8_t *scan_table;

	switch (scan_code) {
	case 0xF0:
		statemachine |= STATE_PRE_RELEASE;
		break;

	case 0xE0:
		statemachine |= STATE_PRE_RIGHT;
		break;

	case 0x11: // alt
		if (statemachine & STATE_PRE_RELEASE)
			kbd.state &= (statemachine & STATE_PRE_RIGHT) ? ~state_LEFT_ALT : ~state_RIGHT_ALT;

		else
			kbd.state |= (statemachine & STATE_PRE_RIGHT) ? state_LEFT_ALT : state_RIGHT_ALT;

		statemachine = STATE_IDLE;
		break;

	case 0x12: // left shift

		if (statemachine & STATE_PRE_RELEASE)
			kbd.state &=  ~state_LEFT_SHIFT;
		else
			kbd.state |= state_LEFT_SHIFT;

		statemachine = STATE_IDLE;
		break;

	case 0x59: //right shift
		if (statemachine & STATE_PRE_RELEASE)
			kbd.state &=  ~state_RIGHT_SHIFT;
		else
			kbd.state |= state_RIGHT_SHIFT;

		statemachine = STATE_IDLE;
		break;

	case 0x14: // control

		if (statemachine & STATE_PRE_RELEASE)
			kbd.state &= (statemachine & STATE_PRE_RIGHT) ? ~state_LEFT_CTRL : ~state_RIGHT_CTRL;
		else
			kbd.state |= (statemachine & STATE_PRE_RIGHT) ? state_LEFT_CTRL : state_RIGHT_CTRL;

		statemachine = STATE_IDLE;
		break;

	case 0x58: // Caps lock

		if (!(statemachine & STATE_PRE_RELEASE))
			BIT_TOGGLE(kbd.state, CAPS_LOCK_BIT);

		statemachine = STATE_IDLE;
		break;

	case 0x77: // Numlock

		if (!(statemachine & STATE_PRE_RELEASE))
			BIT_TOGGLE(kbd.state, NUM_LOCK_BIT);

		statemachine = STATE_IDLE;
		break;

	case 0x7e: // Scroll lock

		if (!(statemachine & STATE_PRE_RELEASE))
			BIT_TOGGLE(kbd.state, SCROLL_LOCK_BIT);

		statemachine = STATE_IDLE;
		break;

	default:

		if (kbd.state & (state_LEFT_SHIFT | state_RIGHT_SHIFT))
			scan_table = kbd_scan_table_shift;
		else
			scan_table = kbd_scan_table;

		if (statemachine == STATE_PRE_RELEASE)
			pr_debug("scan code %hhx  key: %c released\r\n", scan_code, (char) scan_table[scan_code]);
		else {
			pr_debug("scan code %hhx key %c pressed\r\n", scan_code, scan_table[scan_code]);
			if (!write_circ_buffer(kbd.queue, 1, (void *) &scan_table[scan_code]))
				pr_warn("Can't write to keyboard queue, is it full ?\n");
		}

		statemachine = STATE_IDLE;

		break;
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
		(void) kbd_8042_data();

	config = kbd_8042_read_config();
	pr_debug("KBD Initial Config was: %x\r\n", config);

	if (BIT_CHECK(config, 5)) {
		kbd.ps2_dual_channel = true;
		pr_info("Detected dual channel PS2 controller\r\n");
	}

	/* Clean up the config */
	BIT_CLEAR(config, 0); // Disable first PS/2 port interrupt
	BIT_CLEAR(config, 1); // Disable second PS/2 port interrupt
	BIT_CLEAR(config, 6); // Disable first PS/2 port translation

	kbd_8042_write_config(config);
	pr_debug("KBD Config set to: %x\r\n", config);

	if (kbd_8042_self_test())
		return;

	if (ps2_port_test(kbd.ps2_dual_channel))
		return;

	kbd.queue = create_circ_buffer(KBD_BUFFER_SIZE);
	if (!kbd.queue) {
		pr_error("Couldn't get memory for keyboard queue");
		return;
	}


	register_interrupt_handler(IRQ1, &kbd_irq_handler);
	arch_map_irq(1, IRQ1);
	arch_irq_unmask(1);

	kbd_8042_enable(1);
//	kbd_8042_enable(2);
}

static void kbd_8042_enable(uint8_t port)
{
	uint8_t config;
	config = kbd_8042_read_config();

	if (port == 1)
		BIT_SET(config, 0);
	else if (port == 2)
		BIT_SET(config, 1);
	else {
		pr_error("Unknown PS2 port\r\n");
		return;
	}

	kbd_8042_write_config(config);

	if (port == 1)
		kbd_8042_write_cmd(0xAE);
	else
		kbd_8042_write_cmd(0xA8);

}

static void kbd_8042_disable(uint8_t port)
{
	if (port == 1)
		kbd_8042_write_cmd(0xAD);
	else if (port == 2)
		kbd_8042_write_cmd(0xA7);
	else {
		pr_error("Unknown PS2 port\r\n");
		return;
	}

}
static int kbd_8042_self_test()
{
	uint8_t data;
	kbd_8042_write_cmd(0xAA);

	while (!(kbd_8042_status() & STATUS_OUTPUT_BUF_STATUS))
		;

	data = kbd_8042_data();

	if (data != 0x55) {
		pr_error("8042 kbd self test failed\r\n");
		return -1;
	}

	return 0;
}
static int ps2_port_test(bool dual)
{
	uint8_t data;
	kbd_8042_write_cmd(0xAB);

	while (!(kbd_8042_status() & STATUS_OUTPUT_BUF_STATUS))
		;

	data = kbd_8042_data();

	if (data) {
		pr_error("PS2 port 1 test failed\r\n");
		return -1;
	}

	if (!dual)
		return 0;

	kbd_8042_write_cmd(0xA9);

	while (!(kbd_8042_status() & STATUS_OUTPUT_BUF_STATUS))
		;

	data = kbd_8042_data();

	if (data) {
		pr_error("PS2 port 2 test failed\r\n");
		return -1;
	}

	return 0;
}

static void kbd_8042_poll()
{
	uint8_t data;

	while (1) {

		while (!(kbd_8042_status() & STATUS_OUTPUT_BUF_STATUS))
			;

		data = kbd_8042_data();
		printk("Data read from controller: 0x%x\r\n", data);
	}
}

bool kbd_8042_avail()
{
	ACPI_TABLE_FADT *fadt = acpi_get_fadt();

	if (fadt->Header.Revision < 2) {
		pr_debug("ACPI FADT version < 2 - Assuming 8042 availability\r\n");
		return true;
	}

	return (fadt->BootFlags & ACPI_FADT_8042);
}

