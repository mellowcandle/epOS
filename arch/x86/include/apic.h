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

#ifndef APIC_H_W0QIETBW
#define APIC_H_W0QIETBW

#include <lib/list.h>

typedef struct
{
	list_t head;
	uint8_t id;
	addr_t p_addr;
	void *v_addr;
	uint32_t global_irq_base;
	uint8_t max_redirect;
} ioapic_t;

void apic_configure_lapic(uint8_t id, uint8_t processor_id, uint16_t flags);
void apic_configure_ioapic(uint8_t id, addr_t address, addr_t irq_base);
void apic_configure_int_override(uint8_t bus, uint8_t irq_src, uint32_t global_irq, uint16_t flags);
void apic_configure_nmi_source(uint32_t global_irq, uint16_t flags);
void apic_configure_lapic_nmi(uint8_t cpu_id, uint16_t flags, uint8_t lint);
void apic_configure_lapic_override(uint64_t address);

/* check communication with ioapic and reads max redirecton entry*/
void ioapic_santize(ioapic_t *ioapic);


#endif /* end of include guard: APIC_H_W0QIETBW */

