// protos.h - function prototypes
//
//

// for registers_t define
#include "registers.h"

// kernel.c
void fb_blit (void);
void fb_scroll_down (void);
void fb_move_cursor (unsigned short pos);
void fb_clear_con (void);
void loop_mark ();
void kprint (const char *str);
void kprint_newline (void);
void kdelay (uint32_t ticks);
void gdt_flush (void);
void tss_flush (void);
void fb_get_cursor (uint32_t *x, uint32_t *y);
void fb_set_cursor (uint32_t x, uint32_t y);
uint32_t clock (void);

// lib.c
void strreverse(uint8_t* begin, uint8_t* end);
void itoa(int32_t value, uint8_t* str, int32_t base);
uint8_t isNumericChar(uint8_t x);
int32_t atoi(uint8_t *str);
void kprint_int (int32_t n, int32_t base);
int16_t strcmp (const uint8_t * str1, const uint8_t * str2);
uint32_t strlen (const uint8_t *str);
uint8_t *strcpy (uint8_t *dest, uint8_t *src);
uint8_t *strncpy (uint8_t *dest, uint8_t *src, uint32_t len);
uint8_t *memcpy (uint8_t *dest, uint8_t *src, uint32_t count);
uint8_t *memset (uint8_t *dest, uint8_t val, uint32_t count);
uint16_t *memsetw ( uint16_t *dest, uint16_t val, uint32_t count);
uint8_t getch (void);
void run_threads (void);
void run_threads_background (void);

// physmem.c
void pmem_init_bitmap ();
void pmem_set_first_page ();
uint32_t pmem_get_page_from_address (uint32_t address);
int16_t pmem_set_bitmap (uint32_t start, uint32_t end, uint32_t pages, uint8_t state);
int16_t pmem_get_free (size_t size, uint32_t *start, uint32_t *end, uint32_t *pages);
uint32_t pmem_count_free_pages (void);
uint32_t pmem_get_free_ram_info (void);
void *kmalloc (size_t size);
uint32_t kfree (uint32_t address);
void pmem_show_page (uint32_t page);

// thread.c
void thread_message_read (uint8_t *message);
uint32_t thread_number_of_threads (void);
void thread_init(uint32_t baseContextAddress);
void thread_create(uint32_t contextAddress, uint32_t stackStart, uint32_t stackSize, uint32_t entryPoint, uint32_t arg, uint8_t *name);
void thread_exit (uint32_t ret_code);
uint32_t thread_get_own_pid (void);
uint32_t thread_kill (uint32_t pid);
void thread_set_priority (int32_t priority);
void thread_show_info (void);
uint32_t thread_fair_schedule (void);
void thread_schedule(registers_t* oldState);
void thread_saveContext(registers_t* oldState);
void switch_to_user_mode ();
void init_elf (void* image);
void init_multitasking(struct multiboot_info* mb_info);
void get_thread_input_stream (uint8_t *ch);

// message.c
uint8_t message_send (uint8_t *message, uint32_t thread, uint32_t len);
uint8_t message_read (uint8_t *message, uint32_t thread);

// gdt.c
void set_gdt (void);
void set_kernel_stack(uint32_t stack);

// pic.c
void pic_init(void);
void pic_unmask_irq(int intNo);
void pic_mask_irq(int intNo);
void pic_notify(int intNo);

// pit.c
void pit_init(uint32_t frequency);

// physmem.c
void *kmalloc (size_t size);
