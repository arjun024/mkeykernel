// syscall.c -- Defines the implementation of a system call system.
//              Written for JamesM's kernel development tutorials.

#include "interrupts.h"
#include "pic.h"
#include "syscall.h"

#include "types.h"

void kprint_int (int32_t n, int32_t base);
void kprint (const char *str);
void kprint_newline (void);
void fb_set_color (unsigned char forecolor, unsigned char backcolor);
uint8_t getch (void);
uint32_t thread_kill (uint32_t pid);
void pmem_show_page (uint32_t page);
uint32_t pmem_get_free_ram_info (void);
void kdelay (uint32_t ticks);
void get_thread_input_stream (uint8_t ch);
uint8_t message_send (uint8_t *message, uint32_t thread, uint32_t len);
uint8_t message_read (uint8_t *message, uint32_t *thread_sender_pid);
void thread_exit (uint32_t ret_code);

static void syscall_handler(registers_t *regs);

DEFN_SYSCALL1(kprint, 0, const char*);
DEFN_SYSCALL0(kprint_newline, 1);
DEFN_SYSCALL2(kprint_int, 2, int, int);
DEFN_SYSCALL2(fb_set_color, 3, char, char);
DEFN_SYSCALL0(getch, 4);
DEFN_SYSCALL1(thread_kill, 5, uint32_t);
DEFN_SYSCALL1(pmem_show_page, 6, uint32_t);
DEFN_SYSCALL0(pmem_get_free_ram_info, 7);
DEFN_SYSCALL1(kdelay, 8, uint32_t);
DEFN_SYSCALL1(get_thread_input_stream, 9, uint8_t);
DEFN_SYSCALL3(message_send, 10, uint8_t*, uint32_t, uint32_t);
DEFN_SYSCALL2(message_read, 11, uint8_t*, uint32_t);
DEFN_SYSCALL1(thread_exit, 12, uint32_t);

static void *syscalls[13] =
{
    &kprint,
    &kprint_newline,
    &kprint_int,
    &fb_set_color,
    &getch,
    &thread_kill,
    &pmem_show_page,
    &pmem_get_free_ram_info,
    &kdelay,
    &get_thread_input_stream,
    &message_send,
    &message_read,
    &thread_exit
};
uint32_t num_syscalls = 13;

void initialise_syscalls()
{
    // Register our syscall handler.
    interrupts_registerHandler (IRQ_SYSCALL, syscall_handler);
}

void syscall_handler(registers_t *regs)
{
    // Firstly, check if the requested syscall number is valid.
    // The syscall number is found in EAX.

    // kprint ("syscall"); kprint_newline ();

    if (regs->eax >= num_syscalls)
        return;



    // Get the required syscall location.
    void *location = syscalls[regs->eax];

    // We don't know how many parameters the function wants, so we just
    // push them all onto the stack in the correct order. The function will
    // use all the parameters it wants, and we can pop them all back off afterwards.
    int ret;
    asm volatile (" \
      push %1; \
      push %2; \
      push %3; \
      push %4; \
      push %5; \
      call *%6; \
      pop %%ebx; \
      pop %%ebx; \
      pop %%ebx; \
      pop %%ebx; \
      pop %%ebx; \
    " : "=a" (ret) : "r" (regs->edi), "r" (regs->esi), "r" (regs->edx), "r" (regs->ecx), "r" (regs->ebx), "r" (location));
    regs->eax = ret;
}
