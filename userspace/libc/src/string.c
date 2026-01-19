#include <stdint.h>

void* memset(void* dst, int c, uint32_t n) {
    uint8_t* p = (uint8_t*)dst;
    while (n--) {
        *p++ = (uint8_t)c;
    }
    return dst;
}

void* memcpy(void* dst, const void* src, uint32_t n) {
    uint8_t* d = (uint8_t*)dst;
    const uint8_t* s = (const uint8_t*)src;
    while (n--) {
        *d++ = *s++;
    }
    return dst;
}

int memcmp(const void* s1, const void* s2, uint32_t n) {
    const uint8_t* p1 = (const uint8_t*)s1;
    const uint8_t* p2 = (const uint8_t*)s2;
    while (n--) {
        if (*p1 != *p2) {
            return *p1 - *p2;
        }
        p1++;
        p2++;
    }
    return 0;
}

uint32_t strlen(const char* s) {
    uint32_t len = 0;
    while (*s++) {
        len++;
    }
    return len;
}

char* strcpy(char* dst, const char* src) {
    char* ret = dst;
    while ((*dst++ = *src++));
    return ret;
}

char* strncpy(char* dst, const char* src, uint32_t n) {
    char* ret = dst;
    while (n && (*dst++ = *src++)) {
        n--;
    }
    while (n--) {
        *dst++ = '\0';
    }
    return ret;
}

int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

int strncmp(const char* s1, const char* s2, uint32_t n) {
    while (n && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
        n--;
    }
    if (n == 0) {
        return 0;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

char* strchr(const char* s, int c) {
    while (*s) {
        if (*s == (char)c) {
            return (char*)s;
        }
        s++;
    }
    return 0;
}

char* strcat(char* dst, const char* src) {
    char* ret = dst;
    while (*dst) {
        dst++;
    }
    while ((*dst++ = *src++));
    return ret;
}
