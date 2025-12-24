#include "drivers/timer.h"
#include "drivers/io.h"
#include <stdint.h>

#define PIT_CHANNEL0 0x40
#define PIT_COMMAND  0x43
#define PIT_FREQUENCY 1193182

static volatile uint32_t ticks = 0;
static uint32_t timer_frequency = 0;

void timer_callback(void) {
    ticks++;
}

uint32_t get_tick_count(void) {
    return ticks;
}

void timer_init(uint32_t frequency) {
    timer_frequency = frequency;
    uint32_t divisor = PIT_FREQUENCY / frequency;
    
    outb(PIT_COMMAND, 0x36);
    outb(PIT_CHANNEL0, divisor & 0xFF);
    outb(PIT_CHANNEL0, (divisor >> 8) & 0xFF);
}

void sleep(uint32_t ms) {
    if (timer_frequency == 0) {
        return;
    }
    
    uint32_t ticks_needed = (ms * timer_frequency) / 1000;
    if (ticks_needed == 0) ticks_needed = 1;
    
    uint32_t start = ticks;
    uint32_t target = start + ticks_needed;
    
    while (ticks < target) {
        asm volatile("sti; hlt; cli");
    }
}
