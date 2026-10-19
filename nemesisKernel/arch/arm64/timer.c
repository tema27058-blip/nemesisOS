#include <nemesis/timer.h>

u64 timer_frequency(void)
{
    u64 frequency;
    __asm__ volatile("mrs %0, cntfrq_el0" : "=r"(frequency));
    return frequency;
}

u64 timer_ticks(void)
{
    u64 ticks;
    __asm__ volatile("mrs %0, cntvct_el0" : "=r"(ticks));
    return ticks;
}

void timer_init(void)
{
    /* The generic timer is available immediately on QEMU virt. */
}

void delay_ms(u64 milliseconds)
{
    u64 start = timer_ticks();
    u64 required = (milliseconds * timer_frequency()) / 1000;

    while ((timer_ticks() - start) < required) {
        __asm__ volatile("nop");
    }
}
