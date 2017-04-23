/* by Andreas Galauner
 * 
 * https://github.com/G33KatWork
 */

#ifndef _ARCH_REGISTERS_H
#define _ARCH_REGISTERS_H

typedef struct registers
{
	uint32_t gs;
    uint32_t fs;
    uint32_t es;
	uint32_t ds;										// Data segment selector
	uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;	// Pushed by pusha.
	uint32_t int_no, err_code;							// Interrupt number and error code (if applicable)
	uint32_t eip, cs, eflags, useresp, ss;				// Pushed by the processor automatically.
} registers_t;

#endif
