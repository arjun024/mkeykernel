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
#define KERNEL_CODE_SEGMENT_OFFSET 0x08

#define ENTER_KEY_CODE 0x1C

extern unsigned char keyboard_map[128];
extern void keyboard_handler(void);
extern char read_port(unsigned short port);
extern void write_port(unsigned short port, unsigned char data);
extern void load_idt(unsigned long *idt_ptr);

/* fb current cursor location */
unsigned int fb_cursor_x = 0, fb_cursor_y = 0;
unsigned int fb_current_loc = 0;


/* fb text color */
#define FB_BLACK            0x00
#define FB_BLUE             0x01
#define FB_GREEN            0x02
#define FB_CYAN             0x03
#define FB_RED              0x04
#define FB_MAGENTA          0x05
#define FB_BROWN            0x06
#define FB_LIGHT_GREY       0x07
#define FB_DARK_GREY        0x08
#define FB_LIGHT_BLUE       0x09
#define FB_LIGHT_GREEN      0x10
#define FB_LIGHT_CYAN       0x11
#define FB_LIGHT_RED        0x12
#define FB_LIGHT_MAGENTA    0x13
#define FB_LIGHT_BROWN      0x14
#define FB_WHITE            0x0f

unsigned char fb_color = FB_LIGHT_GREY;      /* light grey */

/* video memory begins at address 0xb8000 */
char *vidptr = (char*)0xb8000;

/* frame buffer console */
static unsigned char fb_con[SCREENSIZE];

struct IDT_entry {
	unsigned short int offset_lowerbits;
	unsigned short int selector;
	unsigned char zero;
	unsigned char type_attr;
	unsigned short int offset_higherbits;
};

struct IDT_entry IDT[IDT_SIZE];


void idt_init(void)
{
	unsigned long keyboard_address;
	unsigned long idt_address;
	unsigned long idt_ptr[2];

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
	idt_address = (unsigned long)IDT ;
	idt_ptr[0] = (sizeof (struct IDT_entry) * IDT_SIZE) + ((idt_address & 0xffff) << 16);
	idt_ptr[1] = idt_address >> 16 ;

	load_idt(idt_ptr);
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
        if (str[i] == '\b')
        {
            /*/ backspace */
            
            if (fb_cursor_x > 1)
            {
                fb_cursor_x--;
            }
            else
            {
                if (fb_cursor_y > 0)
                {
                    /* move cursor to previous line end */
                    fb_cursor_y--;
                    fb_cursor_x = COLUMNS_IN_LINE - 1;
                }
            }
            
            fb_current_loc = fb_current_loc - 2;    /* goto old frame buffer position  and clear char */
            fb_con[fb_current_loc] = ' ';
            fb_con[fb_current_loc + 1] = 0x00;
            
            fb_blit ();
        }
        
        if (str[i] != '\b')
        {
            if (fb_cursor_x < COLUMNS_IN_LINE)
            {
                fb_cursor_x++;
            }
            else
            {
                fb_cursor_x = 0;
            
                if (fb_cursor_y < LINES - 1)
                {
                    fb_cursor_y++;
                }
                else
                {
                    fb_scroll_down ();
                    fb_current_loc = SCREENSIZE - (BYTES_FOR_EACH_ELEMENT * COLUMNS_IN_LINE);
                }
            }
            
            fb_con[fb_current_loc] = str[i];
            fb_con[fb_current_loc + 1] = fb_color;
            
            vidptr[fb_current_loc] = str[i];
            vidptr[fb_current_loc + 1] = fb_color;
        
            fb_current_loc = fb_current_loc + 2;
        }
        i++;
        fb_move_cursor (fb_cursor_x + fb_cursor_y * COLUMNS_IN_LINE);
	}
}

void kprint_newline(void)
{
	unsigned int line_size = BYTES_FOR_EACH_ELEMENT * COLUMNS_IN_LINE;
	if (fb_cursor_y < LINES - 1)
    {
        fb_current_loc = fb_current_loc + (line_size - fb_current_loc % (line_size));
        fb_cursor_y++;
        fb_cursor_x = 0;
    }
    else
    {
        fb_scroll_down ();
        fb_current_loc = SCREENSIZE - (BYTES_FOR_EACH_ELEMENT * COLUMNS_IN_LINE);
    }
}

void fb_scroll_down (void)
{
    unsigned int old_pos = BYTES_FOR_EACH_ELEMENT * COLUMNS_IN_LINE, new_pos = 0;
    unsigned int x_pos;
    unsigned int y_pos;
    
    for (y_pos = 1; y_pos < LINES; y_pos++)
    {
        for (x_pos = 0; x_pos < COLUMNS_IN_LINE; x_pos++)
        {
            fb_con[new_pos++] = fb_con[old_pos++];
            fb_con[new_pos++] = fb_con[old_pos++];
        }
    }
    
    new_pos = SCREENSIZE - (BYTES_FOR_EACH_ELEMENT * COLUMNS_IN_LINE);
    /* clear last line */
    for (x_pos = 0; x_pos < COLUMNS_IN_LINE; x_pos++)
    {
        fb_con[new_pos++] = ' ';
        fb_con[new_pos++] = 0x00;
    }
    
    fb_blit ();
}
            
void fb_blit (void)
{
    unsigned int i = 0, j = 0;
	
    while (i < SCREENSIZE)
    {
		vidptr[i++] = fb_con[j++];
		/* vidptr[i++] = fb_con[j++]; */
	}
}    

void fb_clear_screen(void)
{
	unsigned int i = 0;
	while (i < SCREENSIZE) {
		vidptr[i++] = ' ';
		vidptr[i++] = 0x00;
	}
	
	fb_cursor_x = 0; fb_cursor_y = 0;
    fb_current_loc = 0;
    
    fb_clear_con ();
    
    fb_move_cursor (fb_cursor_x + fb_cursor_y * COLUMNS_IN_LINE);
}

void fb_clear_con (void)
{
    unsigned int i = 0;
    while (i < SCREENSIZE) {
		fb_con[i++] = ' ';
		fb_con[i++] = 0x00;
	}
}

void fb_set_color (unsigned char forecolor, unsigned char backcolor)
{
    fb_color = (backcolor << 4) | (forecolor & 0x0F);
}

void keyboard_handler_main(void)
{
	unsigned char status;
	char keycode;
    unsigned char ch[2];

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

		/*
		if (keycode == '\b')
        {
        
            
        }    
		*/
        
		ch[0] = keyboard_map[(unsigned char) keycode];
        ch[1] = '\0';
        
        kprint (ch);
	}
}

void kmain(void)
{
	// const char *str = "red";
	fb_clear_screen();

    fb_set_color (FB_RED, FB_BLACK);
    kprint ("red");
    fb_set_color (FB_BLUE, FB_BLACK);
    kprint (" cube OS");
    fb_set_color(FB_WHITE, FB_BLACK);
    kprint_newline ();
    
    kprint_int (2017);
    kprint_newline ();
    
    /*
    unsigned int line = 1;
    unsigned char ch = 'a';
    unsigned char str[2];
    
    for (line = 1; line < LINES + 1; line++)
    {
        str[0] = ch;
        str[1] = '\0';
        
        kprint (str); kprint_newline();
        ch++;
    }
    */
	idt_init();
	kb_init();

	while(1);
}
