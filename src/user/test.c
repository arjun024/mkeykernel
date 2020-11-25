#include "../kernel/interrupts.h"
#include "../kernel/syscall.h"
#include "../kernel/pit.h"
#include "../kernel/colors.h"
#include "../kernel/message.h"

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

/*
int i = 0;
void _start(void)
{
    syscall_kprint ("Hello from user module!");
    syscall_kprint_newline ();

    while(1);
}
*/

uint8_t getch_sh (void)
{
    uint8_t ch = NULL;

    syscall_kprint ("getch_sh..."); syscall_kprint_newline ();

    do
    {
        syscall_get_thread_input_stream (&ch);
        // syscall_kdelay (50);
        syscall_kprint (">");
        syscall_kprint_int (ch, 10); syscall_kprint_newline ();
    }
    while (ch == NULL);

    return (ch);
}


void itoa(int32_t value, uint8_t* str, int32_t base);
int16_t strcmp (const uint8_t * str1, const uint8_t * str2);

void _startfoo (uint32_t arg)
{
    syscall_kprint ("Welcome to ksh, the kernel shell."); syscall_kprint_newline();
    syscall_thread_exit (0);
}

void _start (uint32_t arg)
{
    uint8_t ch;
    uint8_t command[256];
    uint8_t command_ind = 0;

    command[0] = '\0';

    uint8_t input[256];
    uint8_t input_ind = 0;

    input[0] = '\0';

    uint32_t i, page_start, page_end, pid;
    uint32_t free_mem_kbytes;

    uint8_t msg_event_key = MSG_EVENT_KEY;
    uint8_t msg_reply;
    uint32_t msg_sender;

    syscall_kprint ("Welcome to ksh, the kernel shell."); syscall_kprint_newline();



    while (1)
    {
        syscall_fb_set_color (FB_WHITE, FB_BLACK);
        syscall_kprint ("help = list commands ");
        syscall_fb_set_color (FB_LIGHT_BLUE, FB_BLACK);
        syscall_kprint ("user");
        syscall_fb_set_color (FB_GREEN, FB_BLACK);
        syscall_kprint ("@");
        syscall_fb_set_color (FB_RED, FB_BLACK);
        syscall_kprint ("ksh> ");
        syscall_fb_set_color (FB_WHITE, FB_BLACK);
        command_ind = 0;
        command[0] = '\0';

        kshell_loop:
        // syscall_kdelay (50);

        // goto kshell_loop;

        syscall_message_send (&msg_event_key, 1, 1);
        ch = 0;
        do
        {
            if (syscall_message_read (&msg_reply, &msg_sender) == 0)
            {
                // syscall_kprint ("msg from 0"); syscall_kprint_newline ();

                if (msg_sender == 1)
                {
                    ch = msg_reply;
                }
            }
        } while (ch == 0);
/*
       do
       {
           syscall_get_thread_input_stream (&ch);
           // syscall_kdelay (50);
           // syscall_kprint (">");
           // syscall_kprint_int (ch, 10); syscall_kprint_newline ();
       }
       while (ch == NULL);
*/
        // ch = syscall_getch ();

        syscall_kprint_int (ch, 10);
        if (ch != '\r' && ch != '\b')
        {
            command[command_ind] = ch;
            if (command_ind < 256)
            {
                command_ind++;
                syscall_kdelay (10);
                goto kshell_loop;
            }
        }
        else
        {
            if (ch == '\r')
            {
                // return char, check if command
                command[command_ind] = '\0';

                syscall_kprint ("command: "); syscall_kprint (command); syscall_kprint_newline ();

                if (strcmp (command, "page") == 0)
                {
                    syscall_kprint ("start page block? ");
                    command_ind = 0;

                    input_ind = 0; input[0] = '\0';

                    page_loop:
                    ch = getch_sh ();        // blocking call
                    if (ch != '\r' && ch != '\b')
                    {
                        input[input_ind] = ch;
                        if (input_ind < 256)
                        {
                            input_ind++;
                            goto page_loop;
                        }
                    }
                    input[input_ind] = '\0';
                    page_start = atoi (input);
                    page_end = page_start + 23;

                    for (i = page_start; i <= page_end; i++)
                    {
                        syscall_pmem_show_page (i);
                    }
                }

                if (strcmp (command, "mem") == 0)
                {
                    free_mem_kbytes = syscall_pmem_get_free_ram_info ();
                    syscall_kprint ("free memory: ");
                    syscall_kprint_int (free_mem_kbytes, 10);
                    syscall_kprint (" KB");
                    syscall_kprint_newline ();
                }

                if (strcmp (command, "kill") == 0)
                {
                    syscall_kprint ("pid? ");
                    command_ind = 0;

                    input_ind = 0; input[0] = '\0';

                    kill_loop:
                    ch = getch_sh ();        // blocking call
                    if (ch != '\r' && ch != '\b')
                    {
                        input[input_ind] = ch;
                        if (input_ind < 256)
                        {
                            input_ind++;
                            goto kill_loop;
                        }
                    }
                    input[input_ind] = '\0';
                    pid = atoi (input);

                    if (syscall_thread_kill (pid) == 0)
                    {
                        syscall_kprint ("thread killed!"); syscall_kprint_newline ();
                    }
                }

                if (strcmp (command, "help") == 0)
                {
                    syscall_kprint ("commands: page [list memory pages], mem [list free memory]"); syscall_kprint_newline ();
                    syscall_kprint ("kill [kill thread]"); syscall_kprint_newline ();
                }
            }
            else
            {
                // backspace

                if (command_ind > 0)
                {
                    command_ind = command_ind - 1;
                    command[command_ind] = '\0';
                    goto kshell_loop;
                }
            }
        }
    }
}
