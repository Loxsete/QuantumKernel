#include "lib/string.h"

int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

int strncmp(const char* s1, const char* s2, int n) {
    for (int i = 0; i < n; i++) {
        if (s1[i] != s2[i] || s1[i] == 0 || s2[i] == 0)
            return (unsigned char)s1[i] - (unsigned char)s2[i];
    }
    return 0;
}

int strlen(const char* s) {
    int len = 0;
    while (s[len]) len++;
    return len;
}
