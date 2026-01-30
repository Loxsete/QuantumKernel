#ifndef LIBC_H
#define LIBC_H

#include <stddef.h>
#include <stdint.h>


#ifndef NULL
#define NULL ((void*)0)
#endif


void* memset(void* ptr, int value, uint32_t num);
void* memcpy(void* dest, const void* src, uint32_t n);
int memcmp(const void* s1, const void* s2, uint32_t n);


int atoi(const char* str);
void itoa(int value, char* str, int base);


void* malloc(size_t size);
void  free(void* ptr);


#endif
