#pragma once
#include <stdint.h>

#define TASK_RUNNING  0
#define TASK_SLEEPING 1
#define TASK_ZOMBIE   2

#define TASK_NAME_LEN 32

typedef struct task {
    uint32_t esp;
    uint32_t ebp;
    uint32_t eip;

    uint32_t wakeup_tick;
    int state;
    int pid;

    char name[TASK_NAME_LEN];   

    struct task* next;
} task_t;


void task_init(void);
task_t* task_create(void (*entry)(void), const char* name);
void task_schedule(void);
void task_sleep(uint32_t ms);
void task_exit(void);

task_t* task_current(void);
void task_set_current(task_t* t);
task_t* task_get_list(void);
int task_kill(int pid);
