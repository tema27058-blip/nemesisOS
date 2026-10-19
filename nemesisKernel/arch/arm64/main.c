#include <nemesis/uart.h>
#include <nemesis/print.h>

void kernel_main(void)
{
    uart_init();

    print_line("nemesisOS v0.0.1");
    print_line("nemesisKernel: AArch64 bring-up");

    for (;;) {
        __asm__ volatile("wfe");
    }
}
