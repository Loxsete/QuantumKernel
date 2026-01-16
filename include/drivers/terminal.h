#ifndef FB_TERMINAL_H
#define FB_TERMINAL_H
#include <stdint.h>

#define TERM_COLOR_BG      0x000B001F   
#define TERM_COLOR_FG      0x00FFD9E0   
#define TERM_COLOR_DIM     0x005C2C6D  

#define TERM_COLOR_OK      0x0032FF6A   
#define TERM_COLOR_FAIL    0x00FF3278   
#define TERM_COLOR_WARN    0x00FF7142   
#define TERM_COLOR_INFO    0x0062FFFF   
void term_init(void);
void term_clear(void);
void term_putc(char c);
void term_puts(const char* s);
void term_init_fb(uint32_t addr, uint32_t w, uint32_t h, uint32_t pitch, uint32_t bpp);
void putpixel(uint32_t x, uint32_t y, uint32_t color);
uint32_t getpixel(uint32_t x, uint32_t y);
void draw_mouse(void);
void term_set_color(uint32_t fg);
void term_reset_color(void);

#endif
