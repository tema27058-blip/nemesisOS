#include <nemesis/print.h>
#include <nemesis/uart.h>

static const char hex_digits[] = "0123456789abcdef";

void print_str(const char *s)
{
    uart_puts(s);
}

void print_line(const char *s)
{
    uart_puts(s);
    uart_putc('\n');
}

void print_u64(u64 value)
{
    char buffer[32];
    int index = 0;

    if (value == 0) {
        uart_putc('0');
        return;
    }

    while (value > 0) {
        buffer[index++] = (char)('0' + (value % 10));
        value /= 10;
    }

    while (index > 0) {
        uart_putc(buffer[--index]);
    }
}

void print_hex(u64 value)
{
    int shift;

    for (shift = 60; shift >= 0; shift -= 4) {
        uart_putc(hex_digits[(value >> shift) & 0xf]);
    }
}
