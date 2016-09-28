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
// Register offsets from IOPIC BASE

// Access registers:

#define IOPIC_IOREGSEL	0x00
#define IOPIC_IOWIN		0x10


#define REG_IOAPICID	0x00
#define REG_IOAPICVER	0x01
#define REG_IOAPICARB	0x02


uint32_t iopic_readreg(iopic_t * iopic, addr_t reg)
{
	uint32_t * rsel = iopic->v_addr + IOPIC_IOREGSEL;
	uint32_t * rread = iopic->v_addr + IOPIC_IOWIN;
	*rsel = reg;
	return *rread;
}

void iopic_writereg(iopic_t * iopic, addr_t reg, uint32_t val)
{
	uint32_t * rsel = iopic->v_addr + IOPIC_IOREGSEL;
	uint32_t * rwrite = iopic->v_addr + IOPIC_IOWIN;
	*rsel = reg;
	*rwrite = val;
}


void iopic_test(iopic_t * iopic)
{
	uint32_t id = iopic_readreg(iopic, REG_IOAPICID);
	uint32_t ver = iopic_readreg(iopic, REG_IOAPICVER);
	pr_info("IOPIC ID = %x, Version = %x\r\n", id, ver);
}
