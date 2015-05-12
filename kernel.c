/*
* Copyright (C) 2014  Arjun Sreedharan
* License: GPL version 2 or higher http://www.gnu.org/licenses/gpl.html
*/
#include "keyboard_map.h"

/* there are 25 lines each of 80 columns; each element takes 2 bytes */
#define LINES 25
#define COLUMNS_IN_LINE 80
#define BYTES_FOR_EACH_ELEMENT 2
#define SCREENSIZE BYTES_FOR_EACH_ELEMENT * COLUMNS_IN_LINE * LINES

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64
#define IDT_SIZE 256
#define INTERRUPT_GATE 0x8e
#define KERNEL_CODE_SEGMENT_OFFSET KERNEL_CS

#define GDT_SIZE 3
#define GDT_MEM_LOW 0
#define GDT_MEM_LEN 0xFFFFFFFF

#define GDT_EXE 0x8
#define GDT_READ 0x2
#define GDT_WRITE 0x2

/* Kernel code always runs in ring 0 */
#define DPL_KERNEL 0

/* GDT entry numbers */
enum {
	_GDT_NULL,
	_KERNEL_CS,
	_KERNEL_DS
};

/* GDT entry offsets */
#define GDT_NULL (_GDT_NULL << 3)
#define KERNEL_CS (_KERNEL_CS << 3)
#define KERNEL_DS (_KERNEL_DS << 3)

#define ENTER_KEY_CODE 0x1C

extern unsigned char keyboard_map[128];
extern void keyboard_handler(void);
extern char read_port(unsigned short port);
extern void write_port(unsigned short port, unsigned char data);

/* current cursor location */
unsigned int current_loc = 0;
/* video memory begins at address 0xb8000 */
char *vidptr = (char*)0xb8000;

struct GDT_entry {
	/* Low 8 bits of the "limit", or length of memory this descriptor refers to. */
	unsigned short limit_low;
	unsigned short base_low; /* 'Low' 16-bits of the base */
	unsigned char base_middle; /* 'middle' 8 bits of the base */

	unsigned char type :4; /* Flags for type of memory this descriptor describes */
	unsigned char one :1;
	unsigned char dpl :2; /* Descriptor privilege level - Ring level */
	unsigned char present :1; /* 1 for any valid GDT entry */

	unsigned char limit :4; /* Top 4 bytes of 'limit' */
	unsigned char avilable :1;
	unsigned char zero :1;
	unsigned char op_size :1; /* Selects between 16-bit and 32-bit */
	unsigned char gran :1; /* If this bit is set, then 'limit' is a count of 4K blocks, not bytes */

	unsigned char base_high; /* High 8 bits of the base */
} __attribute__((packed));

struct gdt_ptr {
	unsigned short limit;
	unsigned long base;
} __attribute__((packed));

extern void load_gdt(struct gdt_ptr *gdt_ptr);

#define GDT_ENTRY(gdt_type, gdt_base, gdt_limit, gdt_dpl) \
	{                                                     \
		.limit_low   = (((gdt_limit) >> 12) & 0xFFFF),    \
		.base_low    = ((gdt_base) & 0xFFFF),             \
		.base_middle = (((gdt_base) >> 16) & 0xFF),       \
		.type = gdt_type,                                 \
		.one = 1,                                         \
		.dpl = gdt_dpl,                                   \
		.present = 1,                                     \
		.limit = ((gdt_limit) >> 28),                     \
		.avilable = 0,                                    \
		.zero = 0,                                        \
		.op_size = 1,                                     \
		.gran = 1,                                        \
		.base_high = (((gdt_base) >> 24) & 0xFF),         \
	}

/* Define our GDT that we'll use - We know everything upfront, so we just
 * initalize it with the correct settings.
 *
 * This sets up the NULL, entry, and then the kernel's CS and DS segments,
 * which just span all 4GB of memory. */
struct GDT_entry GDT[GDT_SIZE] = {
	[_GDT_NULL] = { 0 /* NULL GDT entry - Required */ },
	[_KERNEL_CS] = GDT_ENTRY(GDT_EXE | GDT_READ, 0, 0xFFFFFFFF, DPL_KERNEL),
	[_KERNEL_DS] = GDT_ENTRY(GDT_WRITE,          0, 0xFFFFFFFF, DPL_KERNEL)
};

void gdt_init(void)
{
	struct gdt_ptr gdt_ptr;

	gdt_ptr.base = (unsigned long)GDT;
	gdt_ptr.limit = sizeof(GDT);
	load_gdt(&gdt_ptr);
}


struct IDT_entry{
	unsigned short int offset_lowerbits;
	unsigned short int selector;
	unsigned char zero;
	unsigned char type_attr;
	unsigned short int offset_higherbits;
} __attribute__((packed));

struct idt_ptr {
	unsigned short limit;
	unsigned long base;
} __attribute__((packed));

extern void load_idt(struct idt_ptr *idt_ptr);

struct IDT_entry IDT[IDT_SIZE];


void idt_init(void)
{
	unsigned long keyboard_address;
	struct idt_ptr idt_ptr;

	/* populate IDT entry of keyboard's interrupt */
	keyboard_address = (unsigned long)keyboard_handler;
	IDT[0x21].offset_lowerbits = keyboard_address & 0xffff;
	IDT[0x21].selector = KERNEL_CODE_SEGMENT_OFFSET;
	IDT[0x21].zero = 0;
	IDT[0x21].type_attr = INTERRUPT_GATE;
	IDT[0x21].offset_higherbits = (keyboard_address & 0xffff0000) >> 16;

	/*     Ports
	*	 PIC1	PIC2
	*Command 0x20	0xA0
	*Data	 0x21	0xA1
	*/

	/* ICW1 - begin initialization */
	write_port(0x20 , 0x11);
	write_port(0xA0 , 0x11);

	/* ICW2 - remap offset address of IDT */
	/*
	* In x86 protected mode, we have to remap the PICs beyond 0x20 because
	* Intel have designated the first 32 interrupts as "reserved" for cpu exceptions
	*/
	write_port(0x21 , 0x20);
	write_port(0xA1 , 0x28);

	/* ICW3 - setup cascading */
	write_port(0x21 , 0x00);
	write_port(0xA1 , 0x00);

	/* ICW4 - environment info */
	write_port(0x21 , 0x01);
	write_port(0xA1 , 0x01);
	/* Initialization finished */

	/* mask interrupts */
	write_port(0x21 , 0xff);
	write_port(0xA1 , 0xff);

	/* fill the IDT descriptor */
	idt_ptr.limit = sizeof(IDT);
	idt_ptr.base = (unsigned long)IDT;

	load_idt(&idt_ptr);
}

void kb_init(void)
{
	/* 0xFD is 11111101 - enables only IRQ1 (keyboard)*/
	write_port(0x21 , 0xFD);
}

void kprint(const char *str)
{
	unsigned int i = 0;
	while (str[i] != '\0') {
		vidptr[current_loc++] = str[i++];
		vidptr[current_loc++] = 0x07;
	}
}

void kprint_newline(void)
{
	unsigned int line_size = BYTES_FOR_EACH_ELEMENT * COLUMNS_IN_LINE;
	current_loc = current_loc + (line_size - current_loc % (line_size));
}

void clear_screen(void)
{
	unsigned int i = 0;
	while (i < SCREENSIZE) {
		vidptr[i++] = ' ';
		vidptr[i++] = 0x07;
	}
}

void keyboard_handler_main(void) {
	unsigned char status;
	char keycode;

	/* write EOI */
	write_port(0x20, 0x20);

	status = read_port(KEYBOARD_STATUS_PORT);
	/* Lowest bit of status will be set if buffer is not empty */
	if (status & 0x01) {
		keycode = read_port(KEYBOARD_DATA_PORT);
		if(keycode < 0)
			return;

		if(keycode == ENTER_KEY_CODE) {
			kprint_newline();
			return;
		}

		vidptr[current_loc++] = keyboard_map[(unsigned char) keycode];
		vidptr[current_loc++] = 0x07;
	}
}

void kmain(void)
{
	const char *str = "my first kernel with keyboard support";
	clear_screen();
	kprint(str);
	kprint_newline();
	kprint_newline();

	gdt_init();
	idt_init();
	kb_init();

	while(1)
		asm volatile ("hlt");
}

