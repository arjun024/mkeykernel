/* by Andreas Galauner
 * 
 * https://github.com/G33KatWork
 */

#ifndef _ARCH_IDT_H
#define _ARCH_IDT_H

#include "types.h"

// count of gdt entries
#define I86_IDT_MAX_ENTRY_COUNT			255

// idt descriptor type attribute flags
#define I86_IDT_ATTR_TASK_GATE			0x05		//00000101
#define I86_IDT_ATTR_16BIT_INT_GATE		0x06		//00000110
#define I86_IDT_ATTR_16BIT_TRAP_GATE	0x07		//00000111
#define I86_IDT_ATTR_32BIT_INT_GATE		0x0E		//00001110
#define I86_IDT_ATTR_32BIT_TRAP_GATE	0x0F		//00001111

#define I86_IDT_ATTR_SYSTEM_SEGMENT		0x10		//00010000

#define I86_IDT_ATTR_PRIV_KERNEL		0x00		//00000000
#define I86_IDT_ATTR_PRIV_RING1			0x20		//00100000
#define I86_IDT_ATTR_PRIV_RING2			0x40		//01000000
#define I86_IDT_ATTR_PRIV_USER			0x60		//01100000

#define I86_IDT_ATTR_PRESENT			0x80		//10000000

struct idt_entry
{
	uint16_t offset_low;
	uint16_t selector;
	uint8_t zero;
	uint8_t type_attr;
	uint16_t offset_high;
} __attribute__((packed));

struct idt_ptr
{
	uint16_t size;
	uint32_t offset;
} __attribute__((packed));

void idt_flush(void);
void idt_set_gate(uint8_t i, uint32_t base, uint16_t sel, uint8_t flags);
void idt_install(void);

/**
 * ISRs for Exceptions (IRQs 0 to 31) (see interrupt.S)
**/
void isr0(void);
void isr1(void);
void isr2(void);
void isr3(void);
void isr4(void);
void isr5(void);
void isr6(void);
void isr7(void);
void isr8(void);
void isr9(void);
void isr10(void);
void isr11(void);
void isr12(void);
void isr13(void);
void isr14(void);
void isr15(void);
void isr16(void);
void isr17(void);
void isr18(void);
void isr19(void);
void isr20(void);
void isr21(void);
void isr22(void);
void isr23(void);
void isr24(void);
void isr25(void);
void isr26(void);
void isr27(void);
void isr28(void);
void isr29(void);
void isr30(void);
void isr31(void);

/**
 * ISRs for Hardware-IRQs (IRQs 32 to 47) (see interrupt.S)
**/
void irq0(void);
void irq1(void);
void irq2(void);
void irq3(void);
void irq4(void);
void irq5(void);
void irq6(void);
void irq7(void);
void irq8(void);
void irq9(void);
void irq10(void);
void irq11(void);
void irq12(void);
void irq13(void);
void irq14(void);
void irq15(void);

#endif
