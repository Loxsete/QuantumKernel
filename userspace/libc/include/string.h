#ifndef LIBC_STRING_H
#define LIBC_STRING_H

#include <stdint.h>

void* memset(void* dst, int c, uint32_t n);
void* memcpy(void* dst, const void* src, uint32_t n);
int memcmp(const void* s1, const void* s2, uint32_t n);
uint32_t strlen(const char* s);
char* strcpy(char* dst, const char* src);
char* strncpy(char* dst, const char* src, uint32_t n);
int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, uint32_t n);
char* strchr(const char* s, int c);
char* strcat(char* dst, const char* src);

#endif 
