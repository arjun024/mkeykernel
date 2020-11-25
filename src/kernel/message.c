#include "types.h"
#include "message.h"
#include "interrupts.h"

struct message *msg_head = NULL;


uint8_t *memcpy (uint8_t *dest, const uint8_t *src, uint32_t count);
uint32_t thread_get_own_pid (void);

uint8_t message_send (uint8_t *message, uint32_t thread, uint32_t len)
{
    struct message *current_msg = msg_head;
    struct message *next;

    uint32_t  thread_sender_num;

    interrupts_disable ();

    thread_sender_num = thread_get_own_pid ();

    if (msg_head == NULL)
    {
        // no message stored, allocate msg_head

        msg_head = (struct message *) kmalloc (sizeof (struct message));
        if (msg_head == NULL)
        {
            // error nomem
            interrupts_enable ();
            return (1);
        }

        msg_head->data_len = len;
        msg_head->data = kmalloc (len);
        if (msg_head->data == NULL)
        {
            // error nomem
            interrupts_enable ();
            return (1);
        }

        memcpy (msg_head->data, message, len);
        msg_head->thread_num = thread;
        msg_head->data_ready = 1;
        msg_head->next = NULL;
        msg_head->thread_sender_num = thread_sender_num;
    }
    else
    {
        // search next empty message in list
        next = current_msg->next;
        while (next->next != NULL)
        {
            next = next->next;
        }

        next = (struct message *) kmalloc (sizeof (struct message));
        if (next == NULL)
        {
            // error nomem
            interrupts_enable ();
            return (1);
        }

        next->data_len = len;
        next->data = kmalloc (len);
        if (next->data == NULL)
        {
            // error nomem
            interrupts_enable ();
            return (1);
        }

        memcpy (next->data, message, len);
        next->thread_num = thread;
        next->data_ready = 1;
        next->next = NULL;
        msg_head->thread_sender_num = thread_sender_num;
    }
    interrupts_enable ();
    return (0);
}

uint8_t message_read (uint8_t *message, uint32_t *thread_sender_pid)
{
    struct message *current_msg = msg_head;
    struct message *next;

    uint32_t thread_pid;

    interrupts_disable ();

    thread_pid = thread_get_own_pid ();

    next = current_msg->next;

    if (current_msg->thread_num == thread_pid)
    {
        // thread number matches, copy data

        memcpy (message, current_msg->data, current_msg->data_len);
        thread_sender_pid = current_msg->thread_sender_num;
        interrupts_enable ();
        return (0);
    }
    else
    {
        while (next->next != NULL)
        {
            if (next->thread_num == thread_pid)
            {
                // thread number matches, copy data
                memcpy (message, next->data, next->data_len);
                thread_sender_pid = current_msg->thread_sender_num;
                interrupts_enable ();
                return (0);
            }
            next = next->next;
        }
    }

    // message matching thread number not found
    interrupts_enable ();
    return (1);
}
