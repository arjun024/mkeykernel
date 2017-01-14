#include "pic.h"
#include "ports.h"

//-----------------------------------------------
//	Controller Registers
//-----------------------------------------------

// PIC 1 register port addresses
#define I86_PIC1_REG_COMMAND	0x20
#define I86_PIC1_REG_STATUS		0x20
#define I86_PIC1_REG_DATA		0x21
#define I86_PIC1_REG_IMR		0x21

// PIC 2 register port addresses
#define I86_PIC2_REG_COMMAND	0xA0
#define I86_PIC2_REG_STATUS		0xA0
#define I86_PIC2_REG_DATA		0xA1
#define I86_PIC2_REG_IMR		0xA1

//-----------------------------------------------
//	Initialization Command Bit Masks
//-----------------------------------------------

// Initialization Control Word 1 bit masks
#define I86_PIC_ICW1_MASK_IC4			0x1			//00000001
#define I86_PIC_ICW1_MASK_SNGL			0x2			//00000010
#define I86_PIC_ICW1_MASK_ADI			0x4			//00000100
#define I86_PIC_ICW1_MASK_LTIM			0x8			//00001000
#define I86_PIC_ICW1_MASK_INIT			0x10		//00010000

// Initialization Control Words 2 and 3 do not require bit masks

// Initialization Control Word 4 bit masks
#define I86_PIC_ICW4_MASK_UPM			0x1			//00000001
#define I86_PIC_ICW4_MASK_AEOI			0x2			//00000010
#define I86_PIC_ICW4_MASK_MS			0x4			//00000100
#define I86_PIC_ICW4_MASK_BUF			0x8			//00001000
#define I86_PIC_ICW4_MASK_SFNM			0x10		//00010000

//-----------------------------------------------
//	Initialization Command 1 control bits
//-----------------------------------------------

#define I86_PIC_ICW1_IC4_EXPECT				1			//1
#define I86_PIC_ICW1_IC4_NO					0			//0
#define I86_PIC_ICW1_SNGL_YES				2			//10
#define I86_PIC_ICW1_SNGL_NO				0			//00
#define I86_PIC_ICW1_ADI_CALLINTERVAL4		4			//100
#define I86_PIC_ICW1_ADI_CALLINTERVAL8		0			//000
#define I86_PIC_ICW1_LTIM_LEVELTRIGGERED	8			//1000
#define I86_PIC_ICW1_LTIM_EDGETRIGGERED		0			//0000
#define I86_PIC_ICW1_INIT_YES				0x10		//10000
#define I86_PIC_ICW1_INIT_NO				0			//00000

//-----------------------------------------------
//	Initialization Command 4 control bits
//-----------------------------------------------

#define I86_PIC_ICW4_UPM_86MODE			1			//1
#define I86_PIC_ICW4_UPM_MCSMODE		0			//0
#define I86_PIC_ICW4_AEOI_AUTOEOI		2			//10
#define I86_PIC_ICW4_AEOI_NOAUTOEOI		0			//0
#define I86_PIC_ICW4_MS_BUFFERMASTER	4			//100
#define I86_PIC_ICW4_MS_BUFFERSLAVE		0			//0
#define I86_PIC_ICW4_BUF_MODEYES		8			//1000
#define I86_PIC_ICW4_BUF_MODENO			0			//0
#define I86_PIC_ICW4_SFNM_NESTEDMODE	0x10		//10000
#define I86_PIC_ICW4_SFNM_NOTNESTED		0			//a binary 2 (futurama joke hehe ;)

static unsigned char master_mask = 0xFF;
static unsigned char slave_mask = 0xFF;

void pic_init(void)
{
    uint8_t icw = 0;
	
	//Initialization Control Word 1
	icw = (icw & ~I86_PIC_ICW1_MASK_INIT) | I86_PIC_ICW1_INIT_YES;
	icw = (icw & ~I86_PIC_ICW1_MASK_IC4) | I86_PIC_ICW1_IC4_EXPECT;
	outb (I86_PIC1_REG_COMMAND, icw);
	outb (I86_PIC2_REG_COMMAND, icw);
	
	//Initialization Control Word 2
	outb (I86_PIC1_REG_DATA, 0x20);	//First PIC starts at IRQ 0x20
	outb (I86_PIC2_REG_DATA, 0x28);	//Second at 0x28
	
	//Initialization Control Word 3.
	//This is the connection between master and slave.
	//ICW3 for master PIC is the IR that connects to secondary pic in binary format
	//ICW3 for secondary PIC is the IR that connects to master pic in decimal format
	outb (I86_PIC1_REG_DATA, 0x04);
	outb (I86_PIC2_REG_DATA, 0x02);
	
	//Initialization Control Word 4
	//Enables i86 mode
	icw = (icw & ~I86_PIC_ICW4_MASK_UPM) | I86_PIC_ICW4_UPM_86MODE;
	outb (I86_PIC1_REG_DATA, icw);
	outb (I86_PIC2_REG_DATA, icw);
	
	master_mask = 0xFF & ~(1<<2);
	slave_mask = 0xFF;
    outb(I86_PIC1_REG_IMR, master_mask);
    outb(I86_PIC2_REG_IMR, slave_mask);
}
/* by Andreas Galauner
 * 
 * https://github.com/G33KatWork
 */

void pic_unmask_irq(int intNo)
{
    if (intNo > 48 || intNo < 32)
		return;
	
    intNo -= 32;
	
	if(intNo >= 40)     //Slave
	{    
	    slave_mask &= ~(1 << (intNo - 8));
        outb(I86_PIC2_REG_IMR, slave_mask);
	}
	else                //Master
	{
	    master_mask &= ~(1 << intNo);
        outb(I86_PIC1_REG_IMR, master_mask);
	}
}

void pic_mask_irq(int intNo)
{
    if (intNo > 48 || intNo < 32)
		return;
		
	intNo -= 32;
	
	if(intNo >= 40)     //Slave
	{    
	    slave_mask |= 1 << (intNo - 8);
        outb(I86_PIC2_REG_IMR, slave_mask);
	}
	else                //Master
	{
	    master_mask |= 1 << intNo;
        outb(I86_PIC1_REG_IMR, master_mask);
	}
}

void pic_notify(int intNo)
{
    if (intNo > 48 || intNo < 32)
		return;

	if (intNo >= 40)	//Slave
		outb(I86_PIC2_REG_COMMAND, I86_PIC_OCW2_MASK_EOI);

	outb(I86_PIC1_REG_COMMAND, I86_PIC_OCW2_MASK_EOI);
}
