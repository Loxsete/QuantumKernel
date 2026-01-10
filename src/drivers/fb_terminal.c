#include <stdint.h>
#include <stddef.h>
#include "drivers/font.h"

static uint8_t *fb;
static uint32_t fb_width;
static uint32_t fb_height;
static uint32_t fb_pitch;
static uint32_t fb_bpp;
static uint32_t cursor_x;
static uint32_t cursor_y;

void putpixel(uint32_t x, uint32_t y, uint32_t color)
{
    if (x >= fb_width || y >= fb_height) return;

    uint32_t bytes_per_pixel = fb_bpp / 8;
    uint32_t offset = y * fb_pitch + x * bytes_per_pixel;
    uint8_t *p = fb + offset;

    if (fb_bpp == 32)
    {
        p[0] = (color >> 16) & 0xFF;
        p[1] = (color >> 8) & 0xFF;
        p[2] = color & 0xFF;
        p[3] = (color >> 24) & 0xFF;
    }
    else if (fb_bpp == 24)
    {
        p[0] = (color >> 16) & 0xFF;
        p[1] = (color >> 8) & 0xFF;
        p[2] = color & 0xFF;
    }
}

static void draw_char(char ch, uint32_t x, uint32_t y)
{
    const uint8_t *glyph = (const uint8_t*)font8x8_basic[(uint8_t)ch];

    for (uint32_t row = 0; row < FONT_HEIGHT; row++)
    {
        uint8_t line = glyph[row];
        for (uint32_t col = 0; col < FONT_WIDTH; col++)
        {
            if (line & (1 << col))   
                putpixel(x + col, y + row, 0x00FFFFFF);
            else
                putpixel(x + col, y + row, 0x00000000); 
        }
    }
}


void term_putc(char c)
{
    if (c == '\n')
    {
        cursor_x = 0;
        cursor_y += FONT_Y_STEP;
        if (cursor_y + FONT_HEIGHT > fb_height)
            cursor_y = 0;
        return;
    }

    if (c == '\r')
    {
        cursor_x = 0;
        return;
    }

    if ((uint8_t)c < 32) return;

    if (cursor_x + FONT_WIDTH > fb_width)
    {
        cursor_x = 0;
        cursor_y += FONT_Y_STEP;
    }

    if (cursor_y + FONT_HEIGHT > fb_height)
        cursor_y = 0;

    draw_char(c, cursor_x, cursor_y);
    cursor_x += FONT_WIDTH;
}

void term_puts(const char *s)
{
    while (*s)
        term_putc(*s++);
}

void term_clear(void)
{
    for (uint32_t y = 0; y < fb_height; y++)
    {
        uint8_t *row = fb + y * fb_pitch;
        for (uint32_t x = 0; x < fb_width * (fb_bpp / 8); x++)
            row[x] = 0;
    }
    cursor_x = 0;
    cursor_y = 0;
}

void term_init_fb(uint32_t addr, uint32_t w, uint32_t h, uint32_t pitch, uint32_t bpp)
{
    fb = (uint8_t*)(uintptr_t)addr;
    fb_width = w;
    fb_height = h;
    fb_pitch = pitch;
    fb_bpp = bpp;
    term_clear();
}

void term_init(void)
{
    term_clear();
}
