/* by Andreas Galauner
 * 
 * https://github.com/G33KatWork
 */

#ifndef _THREADINFO_H_
#define _THREADINFO_H_

#include "registers.h"

#define THREAD_NAME_LEN 256

#define THREAD_MASTER_ID 0

// signals
#define THREAD_BREAK_PROTECT 1
#define THREAD_BREAK_ALLOWED 0

// requests
#define THREAD_SHUTDOWN      1
#define THREAD_NO_REQUEST    0

// priority
#define THREAD_PRIORITY_NORMAL 0
#define THREAD_PRIORITY_HIGH   10
#define THREAD_PRIORITY_LOW    -5

typedef struct ThreadContext_
{
    uint32_t  eip;       // 0
    uint32_t  cs;        // 4
    uint32_t  eflags;    // 8
    uint32_t  eax;       // 12
    uint32_t  ecx;       // 16
    uint32_t  edx;       // 20
    uint32_t  ebx;       // 24
    uint32_t  esp;       // 28
    uint32_t  ebp;       // 32
    uint32_t  esi;       // 36
    uint32_t  edi;       // 40
    uint32_t  ds;        // 44
    uint32_t  es;        // 48
    uint32_t  fs;        // 52
    uint32_t  gs;        // 56
    uint32_t  ss;        // 60
    uint32_t  cr3;       // 64	- unused for now...
    uint32_t  pid;        // process id
    uint32_t  child_of;   // pid of "master" thread
    int32_t   request;    // OS request for shutdown thread
    uint32_t  signal;     // maybe CTRL-C OR CTRL-D allowed or not?
    int32_t   priority;   // task priority
    uint32_t  next_switch; // next thread switch at this clock
    uint8_t name[THREAD_NAME_LEN];  // name of thread, stored if set at thread create
    uint32_t stack_size;
    uint32_t stack_start;
	struct ThreadContext_* next;
} ThreadContext;

uint32_t thread_getEflags(void);

void thread_init(uint32_t baseContextAddress);
void thread_create(uint32_t infoAddress, uint32_t stackStart, uint32_t stackSize, uint32_t entryPoint, uint32_t arg, uint8_t *name);

void thread_schedule(registers_t* oldState);

void thread_saveContext(registers_t* oldState);
void thread_switchToContext(ThreadContext* newContext);

#endif
