#include <nemesis/exceptions.h>
#include <nemesis/mm.h>
#include <nemesis/print.h>
#include <nemesis/sched.h>
#include <nemesis/timer.h>
#include <nemesis/uart.h>

static u64 task_a_stack[4096];
static u64 task_b_stack[4096];

static void task_a(void)
{
    for (;;) {
        print_line("[task A] running");
        delay_ms(1000);
        sched_yield();
    }
}

static void task_b(void)
{
    for (;;) {
        print_line("[task B] running");
        delay_ms(1000);
        sched_yield();
    }
}

void kernel_main(void)
{
    uart_init();
    exceptions_init();
    timer_init();
    pmm_init();
    sched_init();

    print_line("nemesisOS v0.0.2");
    print_line("nemesisKernel: exceptions, timer, scheduler");

    task_create(task_a, &task_a_stack[4096]);
    task_create(task_b, &task_b_stack[4096]);

    print_line("Starting scheduler...");
    sched_start();
}
