#include <nemesis/exceptions.h>
#include <nemesis/gpu.h>
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

static void boot_splash(void)
{
    fb_clear(0xff0d1117);

    /* nemesisOS logo mark. */
    fb_rect(452, 280, 40, 160, 0xff7c5cff);
    fb_rect(492, 280, 40, 40, 0xff7c5cff);
    fb_rect(532, 320, 40, 120, 0xff7c5cff);

    fb_flush();
    delay_ms(500);

    for (int progress = 0; progress <= 100; progress += 2) {
        fb_rect(312, 520, 400, 10, 0xff1f2430);
        fb_rect(312, 520, progress * 4, 10, 0xff7c5cff);
        fb_flush();
        delay_ms(20);
    }

    delay_ms(400);
}

void kernel_main(void)
{
    uart_init();
    exceptions_init();
    timer_init();
    pmm_init();
    sched_init();

    print_line("nemesisOS v0.0.3");
    print_line("nemesisKernel: graphics bring-up");

    int display = fb_init();

    if (display == 0) {
        print_line("virtio-gpu: framebuffer ready");
        boot_splash();
    } else {
        print_str("virtio-gpu init failed: ");
        print_u64((u64)-display);
        print_line("");
    }

    task_create(task_a, &task_a_stack[4096]);
    task_create(task_b, &task_b_stack[4096]);

    print_line("Starting scheduler...");
    sched_start();
}
