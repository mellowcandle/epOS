#include "kernel/ports.h"

// Write a byte out to the specified port.
void    PORT_outb(uint16_t port, uint8_t value)
{
    __asm volatile ("outb %1, %0" : : "dN" (port), "a" (value));
}

uint16_t PORT_inb(uint16_t port)
{
   uint8_t ret;
   __asm volatile("inb %1, %0" : "=a" (ret) : "dN" (port));
   return ret;
}

uint16_t PORT_inw(uint16_t port)
{
   uint16_t ret;
   __asm volatile ("inw %1, %0" : "=a" (ret) : "dN" (port));
   return ret;
}

