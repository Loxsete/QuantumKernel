#include <stdint.h>
#include <stdarg.h>
#include "syscall.h"
#include "string.h"

void putchar(char c) {
    write(STDOUT, &c, 1);
}

void puts(const char* s) {
    write(STDOUT, s, strlen(s));
    putchar('\n');
}

static void print_num(uint32_t num, int base, int sign) {
    char buf[32];
    int i = 0;
    int is_negative = 0;
    
    if (sign && (int)num < 0) {
        is_negative = 1;
        num = -(int)num;
    }
    
    if (num == 0) {
        buf[i++] = '0';
    } else {
        while (num > 0) {
            int digit = num % base;
            buf[i++] = (digit < 10) ? ('0' + digit) : ('a' + digit - 10);
            num /= base;
        }
    }
    
    if (is_negative) {
        buf[i++] = '-';
    }
    
    
    for (int j = 0; j < i / 2; j++) {
        char tmp = buf[j];
        buf[j] = buf[i - 1 - j];
        buf[i - 1 - j] = tmp;
    }
    
    buf[i] = '\0';
    write(STDOUT, buf, i);
}

int printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    int count = 0;
    
    while (*format) {
        if (*format == '%') {
            format++;
            switch (*format) {
                case 'd':
                case 'i': {
                    int num = va_arg(args, int);
                    print_num(num, 10, 1);
                    break;
                }
                case 'u': {
                    uint32_t num = va_arg(args, uint32_t);
                    print_num(num, 10, 0);
                    break;
                }
                case 'x': {
                    uint32_t num = va_arg(args, uint32_t);
                    print_num(num, 16, 0);
                    break;
                }
                case 's': {
                    char* s = va_arg(args, char*);
                    if (s) {
                        write(STDOUT, s, strlen(s));
                    } else {
                        write(STDOUT, "(null)", 6);
                    }
                    break;
                }
                case 'c': {
                    char c = (char)va_arg(args, int);
                    putchar(c);
                    break;
                }
                case '%': {
                    putchar('%');
                    break;
                }
                default:
                    putchar('%');
                    putchar(*format);
                    break;
            }
            format++;
        } else {
            putchar(*format);
            format++;
        }
        count++;
    }
    
    va_end(args);
    return count;
}
