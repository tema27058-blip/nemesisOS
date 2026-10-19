#ifndef NEMESIS_SCHED_H
#define NEMESIS_SCHED_H

#include <nemesis/types.h>

typedef void (*task_entry_t)(void);

typedef struct {
    u64 x19;
    u64 x20;
    u64 x21;
    u64 x22;
    u64 x23;
    u64 x24;
    u64 x25;
    u64 x26;
    u64 x27;
    u64 x28;
    u64 frame_pointer;
    u64 link_register;
    u64 stack_pointer;
} cpu_context_t;

typedef struct {
    cpu_context_t context;
    task_entry_t entry;
    int used;
} task_t;

void sched_init(void);
void task_create(task_entry_t entry, void *stack_top);
void sched_start(void) __attribute__((noreturn));
void sched_yield(void);

#endif
