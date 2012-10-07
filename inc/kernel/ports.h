#ifndef PORTS_H_INCLUDED
#define PORTS_H_INCLUDED

#include "OS_types.h"

void    PORT_outb(uint16_t port, uint8_t value);
uint16_t PORT_inb(uint16_t port);
uint16_t PORT_inw(uint16_t port);

#endif // PORTS_H_INCLUDED
