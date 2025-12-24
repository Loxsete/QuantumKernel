#ifndef LIBC_H
#define LIBC_H

#include <stddef.h>
#include <stdint.h>

// NULL definition if not provided by stddef.h
#ifndef NULL
#define NULL ((void*)0)
#endif

// Memory functions
void* memset(void* ptr, int value, uint32_t num);
void* memcpy(void* dest, const void* src, uint32_t n);
int memcmp(const void* s1, const void* s2, uint32_t n);

// Conversion functions
int atoi(const char* str);
void itoa(int value, char* str, int base);

#endif
