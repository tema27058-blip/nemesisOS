#include <nemesis/exceptions.h>
#include <nemesis/print.h>

void exception_handler(u64 esr, u64 far)
{
    print_line("KERNEL PANIC: CPU exception");

    print_str("ESR_EL1: 0x");
    print_hex(esr);
    print_line("");

    print_str("FAR_EL1: 0x");
    print_hex(far);
    print_line("");

    for (;;) {
        __asm__ volatile("wfe");
    }
}
