#ifndef STRING_H
#define STRING_H

// String comparison
int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, int n);

// String length
int strlen(const char* s);

// String copy (if needed later)
char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, int n);

// String concatenation (if needed later)
char* strcat(char* dest, const char* src);

#endif
