#include "types.h"

void stack_space (void);

// gdt table
struct gdt_entry_bits
{
	unsigned int limit_low:16;
	unsigned int base_low : 24;
     //attribute byte split into bitfields
	unsigned int accessed :1;
	unsigned int read_write :1; //readable for code, writable for data
	unsigned int conforming_expand_down :1; //conforming for code, expand down for data
	unsigned int code :1; //1 for code, 0 for data
	unsigned int always_1 :1; //should be 1 for everything but TSS and LDT
	unsigned int DPL :2; //priviledge level
	unsigned int present :1;
     //and now into granularity
	unsigned int limit_high :4;
	unsigned int available :1;
	unsigned int always_0 :1; //should always be 0
	unsigned int big :1; //32bit opcodes for code, uint32_t stack for data
	unsigned int gran :1; //1 to use 4k page addressing, 0 for byte addressing
	unsigned int base_high :8;
} __attribute__((packed));

static struct gdt_entry_bits gdt[6];

struct gdt_ptr
{
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

struct gdt_ptr gp;


// tss structure

// A struct describing a Task State Segment.
struct tss_entry_struct
{
   uint32_t prev_tss;   // The previous TSS - if we used hardware task switching this would form a linked list.
   uint32_t esp0;       // The stack pointer to load when we change to kernel mode.
   uint32_t ss0;        // The stack segment to load when we change to kernel mode.
   uint32_t esp1;       // everything below here is unusued now..
   uint32_t ss1;
   uint32_t esp2;
   uint32_t ss2;
   uint32_t cr3;
   uint32_t eip;
   uint32_t eflags;
   uint32_t eax;
   uint32_t ecx;
   uint32_t edx;
   uint32_t ebx;
   uint32_t esp;
   uint32_t ebp;
   uint32_t esi;
   uint32_t edi;
   uint32_t es;
   uint32_t cs;
   uint32_t ss;
   uint32_t ds;
   uint32_t fs;
   uint32_t gs;
   uint32_t ldt;
   uint16_t trap;
   uint16_t iomap_base;
} __packed;

typedef struct tss_entry_struct tss_entry_t;
tss_entry_t tss_entry;

void write_tss(struct gdt_entry_bits *g)
{
   // Firstly, let's compute the base and limit of our entry into the GDT.
   uint32_t base = (uint32_t) &tss_entry;
   uint32_t limit = sizeof(tss_entry);

   // Now, add our TSS descriptor's address to the GDT.
   g->limit_low=limit&0xFFFF;
   g->base_low=base&0xFFFFFF; //isolate bottom 24 bits
   g->accessed=1; //This indicates it's a TSS and not a LDT. This is a changed meaning
   g->read_write=0; //This indicates if the TSS is busy or not. 0 for not busy
   g->conforming_expand_down=0; //always 0 for TSS
   g->code=1; //For TSS this is 1 for 32bit usage, or 0 for 16bit.
   g->always_1=0; //indicate it is a TSS
   g->DPL=3; //same meaning
   g->present=1; //same meaning
   g->limit_high=(limit&0xF0000)>>16; //isolate top nibble
   g->available=0;
   g->always_0=0; //same thing
   g->big=0; //should leave zero according to manuals. No effect
   g->gran=0; //so that our computed GDT limit is in bytes, not pages
   g->base_high=(base&0xFF000000)>>24; //isolate top byte.

   // Ensure the TSS is initially zero'd.
   memset(&tss_entry, 0, sizeof(tss_entry));

   // tss_entry.ss0  = stack_space;  // Set the kernel stack segment.
   // tss_entry.esp0 = stack_space; // Set the kernel stack pointer.

   tss_entry.ss0  = GDT_KERNEL_CODE;  // Set the kernel stack segment.
   tss_entry.esp0 = GDT_KERNEL_DATA;  // Set the kernel stack pointer.

   //note that CS is loaded from the IDT entry and should be the regular kernel code segment
}

void set_kernel_stack(uint32_t stack) //this will update the ESP0 stack used when an interrupt occurs
{
   tss_entry.esp0 = stack;
}


void set_gdt (void)
{
    gp.limit = (sizeof(struct gdt_entry_bits) * 6) - 1;
    gp.base = &gdt;

    //....insert your null_seg 0 segments here or whatever
    struct gdt_entry_bits *null_seg;
    null_seg = (void *) &gdt[0];
    null_seg->limit_low = 0;
    null_seg->base_low = 0;
    null_seg->accessed = 0;
    null_seg->read_write = 0;
    null_seg->conforming_expand_down = 0;
    null_seg->code = 0;
    null_seg->always_1 = 1;
    null_seg->DPL = 0;
    null_seg->present = 0;
    null_seg->limit_high = 0;
    null_seg->available = 0;
    null_seg->big = 0;
    null_seg->gran = 0;
    null_seg->base_high = 0;

    // ring 0
    struct gdt_entry_bits *null_code;
    struct gdt_entry_bits *null_data;
    null_code = (void *) &gdt[1];
    null_data = (void *) &gdt[2];
    null_code->limit_low = 0xFFFF; // 0
    null_code->base_low = 0;
    null_code->accessed = 0;
    null_code->read_write = 1;
    null_code->conforming_expand_down = 0;
    null_code->code = 1;
    null_code->always_1 = 1;
    null_code->DPL = 0;
    null_code->present = 1;
    null_code->limit_high = 0xFFFF;
    null_code->available = 1;
    null_code->big = 1;
    null_code->gran = 1;
    null_code->base_high = 0;
    *null_data = *null_code;
    null_data->code = 0;

    // ring 3
    struct gdt_entry_bits *code;
    struct gdt_entry_bits *data;
    //I assume your ring 0 segments are in gdt[1] and gdt[2] (0 is null segment)
    code=(void*)&gdt[3]; //gdt is a static array of gdt_entry_bits or equivalent
    data=(void*)&gdt[4];
    code->limit_low=0xFFFF;
    code->base_low=0;
    code->accessed=0;
    code->read_write=1; //make it readable for code segments
    code->conforming_expand_down=0; //don't worry about this..
    code->code=1; //this is to signal its a code segment
    code->always_1=1;
    code->DPL=3; //set it to ring 3
    code->present=1;
    code->limit_high=0xFFFF;
    code->available=1;
    code->always_0=0;
    code->big=1; //signal it's 32 bits
    code->gran=1; //use 4k page addressing
    code->base_high=0;
    *data=*code; //copy it all over, cause most of it is the same
    data->code=0; //signal it's not code; so it's data.

    write_tss(&gdt[5]); //we'll implement this function later...

    //...go on to install GDT segments and such
    gdt_flush ();
    //after those are installed we'll tell the CPU where our TSS is:
    tss_flush(); //implement this later
}
