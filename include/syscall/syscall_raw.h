#pragma once
#include <stdint.h>

uint32_t syscall_invoke(
    uint32_t num,
    uint32_t arg1,
    uint32_t arg2,
    uint32_t arg3
);
