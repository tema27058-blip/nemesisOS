#include <nemesis/exceptions.h>
#include <nemesis/framebuffer.h>
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

static void show_boot_animation(void)
{
    framebuffer_clear(0x0b0b12);
    framebuffer_logo(240, 240, 0xffffff, 4);
    framebuffer_text(268, 320, "v0.0.3", 0x8a8a98, 2);

    for (u32 progress = 0; progress <= 100; progress += 2) {
        framebuffer_loading_bar(progress);
        delay_ms(25);
    }

    framebuffer_text(300, 420, "Loading nemesisOS", 0xbcbcc8, 2);
    delay_ms(800);
}

void kernel_main(void)
{
    uart_init();
    exceptions_init();
    timer_init();
    pmm_init();
    framebuffer_init();
    sched_init();

    print_line("nemesisOS v0.0.3");
    print_line("nemesisKernel: framebuffer graphics enabled");

    show_boot_animation();

    task_create(task_a, &task_a_stack[4096]);
    task_create(task_b, &task_b_stack[4096]);

    print_line("Starting scheduler...");
    sched_start();
}
