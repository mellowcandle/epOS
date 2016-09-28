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

#include <apic.h>
#include <printk.h>
#include <kernel/bits.h>

// Access registers offsets

#define IOAPIC_IOREGSEL	0x00
#define IOAPIC_IOWIN		0x10

#define REG_IOAPICID	0x00
#define REG_IOAPICVER	0x01
#define REG_IOAPICARB	0x02
#define REG_IOREDTBL	0x10

#define IOAPIC_VERSION 0x11
/*
typedef struct
{
	union {
		struct {
			int INTVEC		: 8;
			int DELMOD		: 3;
			int DESTMOD		: 1;
			int DELIVS		: 1;
			int INTPOL		: 1;
			int IRR			: 1;
			int TRIGGER_MODE: 1;
			int IRQ_MASK	: 1;
			int				: 32;
			int				: 6;
			int DESTFIELD	: 8;
		};
		uint64_t reg;
	};
} ioapic_redirect_t;
*/
static uint32_t ioapic_readreg(ioapic_t *ioapic, addr_t reg)
{
	uint32_t *rsel = ioapic->v_addr + IOAPIC_IOREGSEL;
	uint32_t *rread = ioapic->v_addr + IOAPIC_IOWIN;
	*rsel = reg;
	return *rread;
}

static void ioapic_writereg(ioapic_t *ioapic, addr_t reg, uint32_t val)
{
	uint32_t *rsel = ioapic->v_addr + IOAPIC_IOREGSEL;
	uint32_t *rwrite = ioapic->v_addr + IOAPIC_IOWIN;
	*rsel = reg;
	*rwrite = val;
}

void ioapic_santize(ioapic_t *ioapic)
{
	uint32_t id = ioapic_readreg(ioapic, REG_IOAPICID);
	uint32_t ver = ioapic_readreg(ioapic, REG_IOAPICVER);

	ioapic->max_redirect = BF_GET(ver, 16, 8);

	if (BF_GET(ver, 0, 7) != IOAPIC_VERSION)
	{
		pr_fatal("IOAPIC version doesn't match.");
		panic();
	}

	pr_info("IOAPIC ID = 0x%x, Version = 0x%x Max redirect = 0x%x\r\n",
	        BF_GET(id, 24, 4), BF_GET(ver, 0, 7), ioapic->max_redirect);
}

void ioapic_apply_redirect(ioapic_t *ioapic)
{
}
