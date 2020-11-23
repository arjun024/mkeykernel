/* by Andreas Galauner
 *
 * https://github.com/G33KatWork
 */

#include "interrupts.h"
#include "pic.h"
#include "protos.h"

static isrFunction isrs[I86_IDT_MAX_ENTRY_COUNT];

void interrupts_init()
{
    memset(isrs, 0, sizeof(isrFunction) * I86_IDT_MAX_ENTRY_COUNT);
}

void interrupts_faultHandler(registers_t regs)
{
    if(isrs[regs.int_no] != 0)
        isrs[regs.int_no](&regs);
    else
    {
        kprint ("Unhandled fault: ");
        kprint_int (regs.int_no, 10);
        kprint_newline ();

		interrupts_disable();
		for(;;);
    }
}

void interrupts_interruptHandler(registers_t regs)
{
    pic_notify(regs.int_no);

    if(isrs[regs.int_no] != 0)
        isrs[regs.int_no](&regs);
    else
    {
        kprint ("Unhandled interrupt: ");
        kprint_int (regs.int_no, 10);
        kprint_newline ();
    }
}

void interrupts_registerHandler(uint8_t i, isrFunction func)
{
    isrs[i] = func;
}

void interrupts_unregisterHandler(uint8_t i)
{
    isrs[i] = 0;
}
