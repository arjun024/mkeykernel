#ifndef _INTERRUPTS_H
#define _INTERRUPTS_H
/* by Andreas Galauner
 *
 * https://github.com/G33KatWork
 */

#include "types.h"
#include "registers.h"
#include "idt.h"

// The following devices use PIC 1 to generate interrupts
#define		IRQ_TIMER			32
#define		IRQ_KEYBOARD		33
#define		IRQ_SERIAL2			34
#define		IRQ_SERIAL1			35
#define		IRQ_PARALLEL2		36
#define		IRQ_DISKETTE		37
#define		IRQ_PARALLEL1		38

// The following devices use PIC 2 to generate interrupts
#define		IRQ_CMOSTIMER		39
#define		IRQ_CGARETRACE		40
#define		IRQ_AUXILIARY		41
#define		IRQ_FPU				42
#define		IRQ_HDC				43

#define     IRQ_SYSCALL         37

// Set the following number to the maximum nested exceptions
// the kernel tries to resolve before it will panic
//#define     MAX_NESTED_EXCEPTIONS   1

typedef void(*isrFunction)(registers_t* regs);

//Handler called by our asm stubs
extern void interrupts_faultHandler(registers_t regs);
extern void interrupts_interruptHandler(registers_t regs);

void interrupts_init(void);

void interrupts_registerHandler(uint8_t i, void (*irsFunction)(registers_t* regs));
void interrupts_unregisterHandler(uint8_t i);

static inline void interrupts_enable(void) __attribute__((always_inline));
static inline void interrupts_enable(void)
{
    asm volatile ("sti");
}

static inline void interrupts_disable(void) __attribute__((always_inline));
static inline void interrupts_disable(void)
{
    asm volatile ("cli");
}

#endif
