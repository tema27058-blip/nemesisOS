#include <nemesis/print.h>
#include <nemesis/uart.h>

void print_str(const char *s)
{
    uart_puts(s);
}

void print_line(const char *s)
{
    uart_puts(s);
    uart_putc('\n');
}

void print_u64(unsigned long long value)
{
    char buf[32];
    int i = 0;

    if (value == 0) {
        uart_putc('0');
        return;
    }

    while (value > 0) {
        buf[i++] = (char)('0' + (value % 10));
        value /= 10;
    }

    while (i > 0) {
        uart_putc(buf[--i]);
    }
}
