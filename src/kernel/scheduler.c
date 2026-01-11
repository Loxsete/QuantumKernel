#include "kernel/task.h"
#include "drivers/timer.h"

extern void context_switch(uint32_t* old, uint32_t* new);

void task_schedule(void) {
    task_t* prev = task_current();
    task_t* next = prev->next;

    uint32_t now = get_tick_count();

    while (next->state != TASK_RUNNING) {
        if (next->state == TASK_SLEEPING && now >= next->wakeup_tick) {
            next->state = TASK_RUNNING;
        }
        next = next->next;
    }

    if (next == prev)
        return;

    task_set_current(next);
    context_switch(&prev->esp, &next->esp);
}
