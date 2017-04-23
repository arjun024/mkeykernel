/* by Andreas Galauner
 * 
 * https://github.com/G33KatWork
 */

#ifndef _PIC_H
#define _PIC_H

#include "types.h"

//-----------------------------------------------
//	Command words are used to control the devices
//-----------------------------------------------

// Command Word 2 bit masks. Use when sending commands
#define		I86_PIC_OCW2_MASK_L1		1		//00000001
#define		I86_PIC_OCW2_MASK_L2		2		//00000010
#define		I86_PIC_OCW2_MASK_L3		4		//00000100
#define		I86_PIC_OCW2_MASK_EOI		0x20	//00100000
#define		I86_PIC_OCW2_MASK_SL		0x40	//01000000
#define		I86_PIC_OCW2_MASK_ROTATE	0x80	//10000000

// Command Word 3 bit masks. Use when sending commands
#define		I86_PIC_OCW3_MASK_RIS		1		//00000001
#define		I86_PIC_OCW3_MASK_RIR		2		//00000010
#define		I86_PIC_OCW3_MASK_MODE		4		//00000100
#define		I86_PIC_OCW3_MASK_SMM		0x20	//00100000
#define		I86_PIC_OCW3_MASK_ESMM		0x40	//01000000
#define		I86_PIC_OCW3_MASK_D7		0x80	//10000000


void pic_init(void);
void pic_unmask_irq(int intNo);
void pic_mask_irq(int intNo);
void pic_notify(int intNo);

#endif
