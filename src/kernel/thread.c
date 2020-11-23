/* by Andreas Galauner
 *
 * https://github.com/G33KatWork
 */

#include "types.h"
#include "thread.h"
#include "interrupts.h"
#include "protos.h"

/*
#include <lib.h>
#include <gdt.h>
#include <interrupts.h>
#include <print.h>
*/

ThreadContext* listHead = NULL;
ThreadContext* currentContext = NULL;

uint32_t thread_number_of_threads (void)
{
    /* get number of current threads */
    uint32_t threads = 0;
    ThreadContext* search = listHead;

    interrupts_disable ();

    while (search->next != NULL)
    {
        threads++;
        search = search->next;
    }

    interrupts_enable ();

    return (threads);
}

void thread_init(uint32_t baseContextAddress)
{
	kprint ("Initializing threading..."); kprint_newline ();
	listHead = currentContext = (ThreadContext*) baseContextAddress;
	memset((uint8_t *) listHead, 0, sizeof(ThreadContext));
	/*print_string_static("Base thread context is at ");
	print_integer_hex((uint32_t)currentContext);
	print_string_static("\n");*/
}

void thread_create(uint32_t contextAddress, uint32_t stackStart, uint32_t stackSize, uint32_t entryPoint, uint32_t arg, uint8_t *name)
{
    // interrupts_disable ();

	ThreadContext* context = (ThreadContext*)contextAddress;
	memset((uint8_t *) context, 0, sizeof(ThreadContext));

	context->cs = GDT_KERNEL_CODE;
	context->ds = GDT_KERNEL_DATA;
	context->es = GDT_KERNEL_DATA;
	context->fs = GDT_KERNEL_DATA;
	context->gs = GDT_KERNEL_DATA;
	context->ss = GDT_KERNEL_DATA;

	context->eflags = thread_getEflags();

	context->eip = entryPoint;

    context->priority = THREAD_PRIORITY_NORMAL;
    context->child_of = currentContext->pid;
    context->pid = (uint32_t) (thread_number_of_threads () + 1);
    context->request = THREAD_NO_REQUEST;
    context->signal = THREAD_BREAK_ALLOWED;
    context->next_switch = 0;

    strncpy (context->name, name, THREAD_NAME_LEN);

	/* We are building this stack here:
     *                     ...
     *          |                        |
     *          | Stackframe of Caller   |  <-- Doesn't exist for now
     *          |------------------------|
     *          |       Parameter        |
     *          |------------------------|
     *          |     Return Address     |  <-- Fake return address for now
     *  ESP --> |------------------------|
     *          | $foo of called function|  <-- Pushed EBP, local variables etc.
     *          |                        |
     *                     ...
     */
	uint32_t* stack = (uint32_t*)(stackStart);
	memset((uint8_t *) stack, 0, stackSize);
	stack[stackSize/sizeof(uint32_t) - 1] = arg;	//Argument
	stack[stackSize/sizeof(uint32_t) - 2] = 0;		//Return Address

	context->esp = stackStart + stackSize - 2*sizeof(uint32_t);
	context->stack_size = stackSize;
	context->stack_start = stackStart;

	ThreadContext* cur = listHead;
	while(cur->next != NULL)
		cur = cur->next;

	cur->next = context;
    // interrupts_enable ();
}

void thread_exit (uint32_t ret_code)
{
    /* exit thread, remove thread from list */
    // ThreadContext* currentContext;
    ThreadContext* search = listHead;

    while (search->next != currentContext)
    {
        search = search->next;
    }

    /* skip current thread in previous thread->next */
    search->next = currentContext->next;

    kfree ((uint32_t) currentContext->esp);
    kfree ((uint32_t) currentContext);
}

uint32_t thread_kill (uint32_t pid)
{
     /* kill thread, remove thread from list */
     uint8_t found_pid = 0;

     ThreadContext* search = listHead;
     ThreadContext* kill;

     interrupts_disable ();

     while (found_pid == 0)
     {
         if (search->pid == pid)
         {
             found_pid = 1;
             kill = search;
         }
         else
         {
             search = search->next;
             if (search->next == NULL)
             {
                 // end of threads list, no given pid found
                 break;
             }
         }
     }

     if (found_pid)
     {
         search = listHead;
         while (search->next != kill)
         {
             search = search->next;
         }

         search->next = kill->next;

         kfree (kill->esp);
         kfree (kill);

         interrupts_enable ();
         return (0);
     }
     else
     {
         interrupts_enable ();
         return (1);
     }
}


void thread_set_priority (int32_t priority)
{
    currentContext->priority = priority;
}


void thread_show_info (void)
{
    /* show thread infos */
    uint8_t run = 1;
    uint32_t threads = 0, mthreads = 0;
    ThreadContext* search = listHead;

    interrupts_disable ();

    do
    {
        kprint ("thread: '"); kprint (search->name); kprint ("' pid: "); kprint_int (search->pid, 10); kprint (", priority: "); kprint_int (search->priority, 10);
        kprint (", child of: "); kprint_int (search->child_of, 10); kprint_newline ();
        threads++;

        if (run == 0 && threads == mthreads) break;
        search = search->next;
        if (search->next == NULL) mthreads = threads + 1, run = 0;

    } while (1);
    kprint ("total threads running: "); kprint_int (threads, 10); kprint_newline ();

    interrupts_enable ();
}

uint32_t thread_fair_schedule (void)
{
    /* get number of current threads */
    uint32_t threads = 0;
    uint32_t ticks_per_sec = 100;
    uint32_t fair_schedule = 0;
    ThreadContext* search = listHead;

    while (search->next != NULL)
    {
        threads++;
        search = search->next;
    }

    if (threads > 0)
    {
        fair_schedule = ticks_per_sec / threads;
        fair_schedule = ticks_per_sec / fair_schedule;
    }

    return (fair_schedule);
}


void thread_schedule(registers_t* oldState)
{
    uint8_t do_schedule = 0;
    uint32_t fair_schedule = thread_fair_schedule ();

	//print_string_static("Scheduling...\n");
	interrupts_disable();

    if (fair_schedule > 0)
    {
        // number of tasks greater as zero -> fair_schedule is not zero
        if (currentContext->next_switch == 0)
        {
            if (currentContext->priority >= 0)
            {
                currentContext->next_switch = clock () + (fair_schedule * currentContext->priority);
            }
            else
            {
                if (currentContext->priority < 0)
                {
                    currentContext->next_switch = clock () + 1;
                }
                else
                {
                    currentContext->next_switch = clock () + fair_schedule;
                }
            }
        }
        else
        {
            if (currentContext->next_switch <= clock ())
            {
                /* time to switch to next task */
                currentContext->next_switch = 0;
                do_schedule = 1;
            }
        }
    }

    if (do_schedule == 1)
    {
        thread_saveContext(oldState);
        //print_string_static("Old context saved...\n");

        ThreadContext* next;
        if(currentContext->next == NULL)
            next = listHead;
        else
            next = currentContext->next;

        currentContext = next;
        /*print_string_static("New thread context is at ");
        print_integer_hex((uint32_t)currentContext);
        print_string_static("\n");*/

        thread_switchToContext(currentContext);
    }
}


void thread_saveContext(registers_t* oldState)
{
	if((oldState->cs & 0x3) == 0x3)			//are we coming from usermode?
    {
		//ss and esp are only valid if we are coming from usermode
        currentContext->ss = oldState->ss;
        currentContext->esp = oldState->useresp;
    }
    else
    {
        currentContext->ss = GDT_KERNEL_DATA;
        currentContext->esp = oldState->esp + 0x14;
    }

    currentContext->eip = oldState->eip;
    currentContext->cs = oldState->cs;
    currentContext->eflags = oldState->eflags;
    currentContext->eax = oldState->eax;
    currentContext->ecx = oldState->ecx;
    currentContext->edx = oldState->edx;
    currentContext->ebx = oldState->ebx;

    currentContext->ebp = oldState->ebp;
    currentContext->esi = oldState->esi;
    currentContext->edi = oldState->edi;
    currentContext->ds = oldState->ds;
    currentContext->es = oldState->es;
    currentContext->fs = oldState->fs;
    currentContext->gs = oldState->gs;
}


void switch_to_user_mode ()
{
    // Set up our kernel stack.
    set_kernel_stack(GDT_KERNEL_DATA);

    // Set up a stack structure for switching to user mode.
    asm volatile("  \
      cli; \
      mov $0x23, %ax; \
      mov %ax, %ds; \
      mov %ax, %es; \
      mov %ax, %fs; \
      mov %ax, %gs; \
                    \
       \
      mov %esp, %eax; \
      pushl $0x23; \
      pushl %esp; \
      pushf; \
      pushl $0x1B; \
      push $1f; \
      iret; \
    1: \
      ");


}
