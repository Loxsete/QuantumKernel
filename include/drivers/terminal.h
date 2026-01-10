#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdint.h>

void term_init(void);
void term_clear(void);
void term_putc(char c);
void term_puts(const char* s);
void term_process_char(char c);
void putpixel(uint32_t x, uint32_t y, uint32_t color);  
void term_init_fb(uint32_t addr, uint32_t w, uint32_t h, uint32_t pitch, uint32_t bpp);
#endif
