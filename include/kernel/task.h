#ifndef TASK_H
#define TASK_H

#include <stdint.h>
#include "cpu/paging.h"

#define TASK_NAME_LEN 32

typedef enum {
    TASK_RUNNING,
    TASK_SLEEPING,
    TASK_ZOMBIE
} task_state_t;

typedef struct task {
    int pid;
    uint32_t esp;
    uint32_t ebp;
    uint32_t eip;
    task_state_t state;
    uint32_t wakeup_tick;
    char name[TASK_NAME_LEN];
    page_context_t* page_ctx;
    struct task* next;
} task_t;

void task_init(void);
task_t* task_create(void (*entry)(void), const char* name);
void task_schedule(void);
task_t* task_current(void);
void task_set_current(task_t* t);
void task_exit(void);
void task_sleep(uint32_t ms);
task_t* task_get_list(void);
int task_kill(int pid);

#endif
