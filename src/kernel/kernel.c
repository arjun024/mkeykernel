/*
* Copyright (C) 2014  Arjun Sreedharan
* License: GPL version 2 or higher http://www.gnu.org/licenses/gpl.html
*/
#include "types.h"
#include "registers.h"
#include "multiboot.h"
#include "idt.h"
#include "ports.h"
#include "interrupts.h"
#include "keyboard_map.h"

void jump_usermode (void);

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

/* kernel debugging mode */
uint8_t debug_mode = 0;
uint8_t command_mode = 0;
uint8_t command[256];
int16_t com_ind = 0;


/* memory */
uint32_t mem_start_address = 0x100000;
uint32_t mem_end_address;
uint32_t mem_use_address = 0x400000;


extern uint8_t keyboard_map[128];
extern uint8_t keyboard_shift_map[128];

extern void load_idt(unsigned long *idt_ptr);

void timerHandler(registers_t* regs);
void thread(uint32_t argument);

extern uint8_t keyboard_shift;

/* fb current cursor location */
uint16_t fb_cursor_x = 0, fb_cursor_y = 0;
uint32_t fb_current_loc = 0;

/* timer */
static uint32_t clock_ticks = 0;


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

uint8_t fb_color = FB_LIGHT_GREY;      /* light grey */

/* video memory begins at address 0xb8000 */
static int8_t *vidptr = (int8_t*)0xb8000;

/* frame buffer console */
uint8_t fb_con[SCREENSIZE];


typedef struct multiboot_memory_map {
	uint32_t size;
    uint64_t base_addr;  
    uint64_t length;
	uint32_t type;
} multiboot_memory_map_t;


// threading 
uint8_t threading = 0;
uint8_t threads_request = 0;

void kb_init(void)
{
	/* 0xFD is 11111101 - enables only IRQ1 (keyboard)*/
	outb(0x21 , 0xFC);
}



void kprint(const char *str)
{
	uint32_t i = 0;
    
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
                    fb_cursor_x = COLUMNS_IN_LINE;
                }
            }
            
            fb_current_loc = fb_current_loc - 2;    /* goto old frame buffer position  and clear char */
            fb_con[fb_current_loc] = ' ';
            fb_con[fb_current_loc + 1] = 0x00;
            
            fb_blit ();
        }
        else
        {
            if (fb_cursor_x < COLUMNS_IN_LINE - 1)
            {
                fb_cursor_x = fb_cursor_x + 1;
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

        fb_move_cursor ((fb_cursor_x + 1) + fb_cursor_y * COLUMNS_IN_LINE);
        
        /* show cursor line on next position */
        vidptr[fb_current_loc] = '_';
        vidptr[fb_current_loc + 1] = fb_color;
	}
}

void kprint_newline(void)
{
	uint16_t line_size = BYTES_FOR_EACH_ELEMENT * COLUMNS_IN_LINE;
    
    /* delete cursor line */
    vidptr[fb_current_loc] = ' ';
    vidptr[fb_current_loc + 1] = fb_color;
    
	if (fb_cursor_y < LINES - 1)
    {
        // fb_current_loc = fb_current_loc + (line_size - (fb_current_loc % line_size));
        
        fb_current_loc = fb_current_loc + (COLUMNS_IN_LINE - fb_cursor_x) * BYTES_FOR_EACH_ELEMENT;
        fb_cursor_y++;
        fb_cursor_x = 0;
        
        fb_move_cursor (fb_cursor_x + fb_cursor_y * COLUMNS_IN_LINE);
        
        fb_blit ();
        
        /* show cursor line on next position */
        vidptr[fb_current_loc ] = '_';
        vidptr[fb_current_loc + 1] = fb_color;
    }   
    else
    {
        fb_scroll_down ();
        fb_current_loc = SCREENSIZE - line_size;
        fb_cursor_x = 0;
    }
}

void fb_scroll_down (void)
{
    uint16_t old_pos = BYTES_FOR_EACH_ELEMENT * COLUMNS_IN_LINE, new_pos = 0;
    uint16_t x_pos;
    uint16_t y_pos;
    
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
    uint16_t i = 0, j = 0;
	
    while (i < SCREENSIZE)
    {
		vidptr[i++] = fb_con[j++];
		/* vidptr[i++] = fb_con[j++]; */
	}
}    

void fb_clear_screen(void)
{
	uint16_t i = 0;
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
    uint16_t i = 0;
    while (i < SCREENSIZE) {
		fb_con[i++] = ' ';
		fb_con[i++] = 0x00;
	}
}

void fb_set_color (unsigned char forecolor, unsigned char backcolor)
{
    fb_color = (backcolor << 4) | (forecolor & 0x0F);
}

void div_by_zero_handler_main (void)
{
    fb_set_color (FB_RED, FB_BLACK);
    kprint ("KERNEL PANIC! division by ZERO"); kprint_newline ();
    fb_set_color (FB_WHITE, FB_BLACK);
    
    outb (0x20, 0x20);
}

void div_by_zero ()
{
    uint8_t i;
    
    i = 23 / 0;
}

void keyboard_handler(registers_t* regs)
{
	uint8_t status;
	uint8_t keycode;
    uint8_t ch[2];
    uint8_t pressed = 0;

    /* debug */
    uint32_t i;
    
    
	status = inb(KEYBOARD_STATUS_PORT);
	/* Lowest bit of status will be set if buffer is not empty */
	if (status & 0x01) {
		keycode = inb(KEYBOARD_DATA_PORT);
		if(keycode < 0)
			return;

		if(keycode == ENTER_KEY_CODE) {
			kprint_newline();
            
            if (command_mode)
            {
                command[com_ind] = '\0';
                
                kprint (command); kprint ("> ");
                if (strcmp (command, "div") == 0)
                {
                    div_by_zero ();
                }
                
                if (strcmp (command, "page") == 0)
                {
                    command[0] = '\0'; com_ind = 0;
                
                    for (i = 0; i < LINES -1; i++)
                    {
                        pmem_show_page (i);
                    }    
                }
                
                if (strcmp (command, "tasks") == 0)
                {
                    command[0] = '\0'; com_ind = 0;
                
                    threads_request = 1;
                }
                
                command[0] = '\0';
                com_ind = 0;
            }
            
			return;
		}

		if (keycode == '\b')
        {
            // backspace
            
            if (command_mode == 1)
            {
                if (com_ind >= 2)
                {
                    com_ind = com_ind - 2;
                    command[com_ind] = '\0';
                }
            }
        }    
        
        if (keycode & 0x80)
        {
            pressed = 0;
        }
        else
        {
            pressed = 1;
        }
    
        if (keycode == KEY_SCAN_ESCAPE)
        {
            /* switch debug mode on/off */
            if (debug_mode == 0)
            {
                debug_mode = 1;
            }
            else
            {
                debug_mode = 0;
            }
        }
        
        if (keycode == KEY_SCAN_F1 || keycode == KEY_SCAN_CTRL_LEFT)
        {
            /* switch command console on */
            
            if (command_mode == 0)
            {
                command_mode = 1;
                
                fb_set_color (FB_RED, FB_BLACK);
                kprint ("COMMAND MODE"); kprint_newline ();
                kprint ("press F1 to EXIT"); kprint_newline ();
                kprint ("commands:"); kprint_newline();
                kprint ("div [division by zero interrupt check]"); kprint_newline ();
                kprint ("page [show usage of pages]"); kprint_newline ();
                kprint ("tasks [multitasking demo]"); kprint_newline ();
                kprint ("press ENTER for prompt!"); kprint_newline ();
            }
            else
            {
                command_mode = 0;
                command[0] = '\0'; com_ind = 0;
                fb_set_color (FB_WHITE, FB_BLACK);
            }
        }
            
        
        if (debug_mode)
        {
            kprint_int (keycode, 16);
            kprint_newline ();
        }
        else
        {
            if (keycode == KEY_SCAN_LEFT_SHIFT || keycode == KEY_SCAN_RIGHT_SHIFT)
            {
                /* uppercase table */
                keyboard_shift = 1;
            }
        
            if (keycode == KEY_SCAN_LEFT_SHIFT_RELEASED || keycode == KEY_SCAN_RIGHT_SHIFT_RELEASED)
            {
                /* lowercase table */
                keyboard_shift = 0;
            }
          
            if (pressed == 1)
            {
                if (keyboard_shift == 1)
                {
                    ch[0] = keyboard_shift_map[(unsigned char) keycode];
                    
                    if (command_mode)
                    {
                        command[com_ind] = ch[0];
                        com_ind++;
                    }
                }
                else
                {
                    ch[0] = keyboard_map[(unsigned char) keycode];
                    
                    if (command_mode)
                    {
                        command[com_ind] = ch[0];
                        com_ind++;
                    }
                }
		
                ch[1] = '\0';
                kprint (ch);
            }
        }
	}
}

uint32_t clock (void)
{
    return (clock_ticks);
}

void kdelay (uint32_t ticks)
{
    uint32_t end_ticks = clock () + ticks;
    
    while (clock () < end_ticks)
    {
    }
}


loop_mark ()
{
    /* simple loop "benchmark" */
    
    uint32_t start_time, end_time, loop_time, i, j, k;
    uint32_t loop_max = 20000;
    uint32_t loops_per_sec;
    
     kprint ("loop mark: ");
    
    start_time = clock ();
    
    for (i = 1; i <= loop_max; i++)
    {
        for (j = 1; j <= loop_max; j++)
        {
            
        }
    }
    
    end_time = clock ();
    loop_time = end_time - start_time;
    
    loops_per_sec = (loop_max * loop_max) / (loop_time / TIMER_FREQ); 
    
    kprint_int (loop_time, 10); kprint_newline ();
    kprint ("loops per sec: "); kprint_int (loops_per_sec, 10); kprint_newline (); kprint_newline ();
}
    
 
void timerHandler(registers_t* regs)
{
    clock_ticks++;
	
    if (threading)
    {
        thread_schedule(regs);
    }
    // kprint ("clock: "); kprint_int (clock_ticks, 10); kprint_newline ();
}
 
void thread_a(uint32_t argument)
{
    uint32_t ticks;
	for(;;)
	{
        ticks = clock ();
		kprint ("thread A: "); kprint_int (ticks, 10); kprint_newline ();
		kdelay (50);
	}
}
  
void thread_b(uint32_t argument)
{
    uint32_t ticks;
	for(;;)
	{
        ticks = clock ();
		kprint ("thread B: "); kprint_int (ticks, 10); kprint_newline ();
		kdelay (50);
	}
}
  
void run_threads (void)
{
    uint8_t *thread_base = (uint8_t *) kmalloc (20000);
    uint8_t *thread_a_context = (uint8_t *) kmalloc (4096);
    uint8_t *thread_a_stack = (uint8_t *) kmalloc (4096);

    uint8_t *thread_b_context = (uint8_t *) kmalloc (4096);
    uint8_t *thread_b_stack = (uint8_t *) kmalloc (4096);
    
    thread_init(thread_base);
	thread_create(thread_a_context, thread_a_stack, 4096, (uint32_t)thread_a, 0x61);
    thread_create(thread_b_context, thread_b_stack, 4096, (uint32_t)thread_b, 0x61);
    
    threading = 1;
}
    
  
void kmain (multiboot_info_t* mbt, unsigned int magic)
{
    set_gdt ();         /* init memory GDT */
    
    idt_install();				//set up IDT
	interrupts_init();			//set up callback table
	pic_init();					//set up PIC
    interrupts_enable();
    interrupts_registerHandler(IRQ_TIMER, timerHandler);
    interrupts_registerHandler(IRQ_KEYBOARD, keyboard_handler);
	pit_init(100);
    
    
    
    
    // vmem_paging ();      /* switch paging on */
    
    multiboot_memory_map_t* mmap = mbt->mmap_addr;
    uint32_t mem_start,  mem_end;
    uint32_t mem_type;
    uint8_t mem_found = 0;
    int16_t mem;
    
    uint32_t pages_free;
    uint64_t ram_free;
    command[0] = '\0';
    
	fb_clear_screen();
    
    fb_set_color(FB_GREEN, FB_BLACK);
    kprint ("level 0: RUN"); kprint_newline ();
    kprint ("level 1: memory"); kprint_newline ();

    pmem_init_bitmap ();
    
	while(mmap < mbt->mmap_addr + mbt->mmap_length) 
    {
        mem_start = mmap->base_addr;
        mem_end = mem_start + mmap->length;
        mem_type = mmap->type;
        
        if (mem_type == 1)
        {
            kprint ("RAM at "); kprint_int ((uint32_t) mmap->base_addr, 16); 
        
            if (mmap->length < 1024 * 1024)
            {
                kprint (" size: "); kprint_int ((uint32_t) mmap->length, 10); kprint (" bytes ");
            }
            else
            {
                kprint (" size: "); kprint_int ((uint32_t) mmap->length / (1024 * 1024), 10); kprint (" MB ");
            }
            kprint ("type: "); kprint_int ((uint32_t) mmap->type, 16);
       
            if (mem_start >= mem_start_address)
            {
                mem_end_address = mem_end;
                mem_found = 1;
                
                mem = pmem_set_bitmap (mem_use_address, mem_end, FREE);
                if (mem == MEM_ERR_OK)
                {
                    kprint (" found base");
                }
                else
                {
                    kprint (" MEMORY ERROR: ");
                    
                    if (mem == MEM_ERR_RANGE)
                    {
                        kprint ("range");
                    }
                    
                    if (mem == MEM_ERR_BAD_MEM || mem == MEM_ERR_DOUBLE_FREE)
                    {
                        kprint ("alloc error");
                    }
                }
                kprint_newline (); 
            }
            else
            {    
                if (mem_start == 0)
                {
                    mem_start = (uint32_t) 0x1000;       /* skip first page */
                }
                        
                mem = pmem_set_bitmap (mem_start, mem_end, FREE);
                if (mem == MEM_ERR_OK)
                {
                    kprint (" free");
                }
                else
                {
                    kprint (" MEMORY ERROR: ");
                    
                    if (mem == MEM_ERR_RANGE)
                    {
                        kprint ("range");
                    }
                        
                    if (mem == MEM_ERR_BAD_MEM || mem == MEM_ERR_DOUBLE_FREE)
                    {
                        kprint ("alloc error");
                    } 
                }
                kprint_newline ();
            }
        }
    
		mmap = (multiboot_memory_map_t*) ( (uint32_t)mmap + mmap->size + sizeof(mmap->size) );
	}
	
	pmem_set_first_page ();    /* so no null pointer for free mem block can exist */
	
	mem = pmem_set_bitmap ((uint32_t) 0xF00000, (uint32_t) 0xFFFFFF, ALLOCATE);
    if (mem != MEM_ERR_OK)
    {
        kprint ("MEMORY ERROR: mark reserved");
        kprint_newline ();
    }
    
    pages_free = pmem_count_free_pages ();
    ram_free = (pages_free * 4096) / 1024 /1024;
    kprint ("free pages: "); kprint_int (pages_free, 10); kprint (" = " ); kprint_int ((uint32_t) ram_free, 10); kprint (" MB"); kprint_newline ();
    
    kprint_newline ();
    kprint ("level 2: interrupts"); kprint_newline ();
    
    // kb_init();
    pic_unmask_irq(IRQ_KEYBOARD);
	
    
    
    kprint ("level 3: keyboard"); kprint_newline ();
    kprint_newline ();
    
    // loop_mark ();
    
    fb_set_color (FB_RED, FB_BLACK);
    kprint ("red");
    fb_set_color (FB_BLUE, FB_BLACK);
    kprint (" cube OS  ");
    fb_set_color(FB_WHITE, FB_BLACK);
    
    kprint_int (2017, 10);
    kprint_newline ();
    kprint_newline ();
    
    uint32_t i;
    
    /*
    for (i = 0; i < 10; i++)
    {
        pmem_show_page (i);
    }
    */
    
    
    /*
    uint8_t *buf;
    buf = (uint8_t *) kmalloc (100000);
    if (buf != NULL)
    {
        kprint ("mem allocated at "); kprint_int (buf, 16); kprint_newline ();
    }
    else
    {
        kprint ("mem allocate failed");
    }
    kprint_newline ();
    
    
    if (kfree (buf) != NULL)
    {
        kprint ("free ERROR!"); kprint_newline ();
    }
    */
    
    kprint ("F1 or CTRL-LEFT = command console"); kprint_newline();
    kprint_newline ();
    
    kprint ("READY"); kprint_newline ();
    
    pic_unmask_irq(IRQ_TIMER);
    
    
    // jump_usermode ();
    
	while(1)
    {
        kdelay (200);
        
        if (threads_request == 1)
        {
            run_threads();
            threads_request = 0;
        }
    }
}
