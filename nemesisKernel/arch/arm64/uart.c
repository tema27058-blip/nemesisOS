#include <nemesis/uart.h>

/* QEMU virt PL011 UART. */
#define UART0_BASE 0x09000000UL
#define UART_DR    (*(volatile unsigned int *)(UART0_BASE + 0x000))
#define UART_FR    (*(volatile unsigned int *)(UART0_BASE + 0x018))
#define UART_FR_TXFF (1UL << 5)

void uart_init(void)
{
    /* QEMU's PL011 is usable immediately after reset. */
}

void uart_putc(char c)
{
    while (UART_FR & UART_FR_TXFF) {
        /* Wait until the transmit FIFO has space. */
    }

    if (c == '\n') {
        UART_DR = (unsigned int)'\r';
    }

    UART_DR = (unsigned int)c;
}

void uart_puts(const char *s)
{
    while (*s) {
        uart_putc(*s++);
    }
}
