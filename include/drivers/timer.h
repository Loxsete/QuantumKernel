#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

extern void irq0_handler(void);

void timer_init(uint32_t frequency);
uint32_t get_tick_count(void);
void sleep(uint32_t ms);

#endif
