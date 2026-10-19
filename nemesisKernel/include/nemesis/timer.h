#ifndef NEMESIS_TIMER_H
#define NEMESIS_TIMER_H

#include <nemesis/types.h>

void timer_init(void);
u64 timer_frequency(void);
u64 timer_ticks(void);
void delay_ms(u64 milliseconds);

#endif
