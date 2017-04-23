/* by Andreas Galauner
 * 
 * https://github.com/G33KatWork
 */

#ifndef _PORTS_H
#define _PORTS_H

#include "types.h"

static inline void outb(unsigned short port, unsigned char value);
static inline void outb(unsigned short port, unsigned char value)
{
    asm volatile ("outb %1, %0" : : "dN" (port), "a" (value));
}

static inline unsigned char inb(unsigned short port);
static inline unsigned char inb(unsigned short port)
{
    unsigned char ret;
    asm volatile ("inb %1, %0" : "=a" (ret) : "dN" (port));
    return ret;
}

static inline unsigned short inw(unsigned short port);
static inline unsigned short inw(unsigned short port)
{
    unsigned short ret;
    asm volatile ("inw %1, %0" : "=a" (ret) : "dN" (port));
    return ret;
}

#endif
