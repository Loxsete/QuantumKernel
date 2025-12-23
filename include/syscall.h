#pragma once
#include <stdint.h>

#define SYS_WRITE 1
#define SYS_EXIT  2

int write(int fd, const char* buf, uint32_t len);
void exit(void);
