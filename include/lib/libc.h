#ifndef LIBC_H
#define LIBC_H

#include <stdint.h>

int memcmp(const void* s1, const void* s2, uint32_t n);
void* memset(void* ptr, int value, uint32_t num);
void* memcpy(void* dest, const void* src, uint32_t n);
int atoi(const char* str);
void itoa(int value, char* str, int base);

#endif
