#ifndef NEMESIS_UART_H
#define NEMESIS_UART_H

void uart_init(void);
void uart_putc(char c);
void uart_puts(const char *s);

#endif
