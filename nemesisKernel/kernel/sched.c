#include <nemesis/sched.h>
#include <nemesis/timer.h>

#define MAX_TASKS 4

extern void task_switch(task_t *previous, task_t *next);

static task_t tasks[MAX_TASKS];
static int task_count;
static int current_task;

void sched_init(void)
{
    task_count = 0;
    current_task = -1;
}

void task_create(task_entry_t entry, void *stack_top)
{
    if (task_count >= MAX_TASKS) {
        return;
    }

    task_t *task = &tasks[task_count++];
    task->entry = entry;
    task->used = 1;
    task->context.link_register = (u64)entry;
    task->context.stack_pointer = (u64)stack_top;
}

void sched_yield(void)
{
    int next = (current_task + 1) % task_count;
    task_t *previous = &tasks[current_task];

    current_task = next;
    task_switch(previous, &tasks[next]);
}

void sched_start(void)
{
    task_t boot_context;

    current_task = 0;
    task_switch(&boot_context, &tasks[0]);

    for (;;) {
        __asm__ volatile("wfe");
    }
}
