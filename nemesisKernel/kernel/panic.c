#include <nemesis/panic.h>
#include <nemesis/print.h>
#include <nemesis/uart.h>

void panic(const char *reason)
{
    print_line("KERNEL PANIC");
    print_line(reason);

    for (;;) {
        __asm__ volatile("wfe");
    }
}
