#include <stdint.h>
#include <stddef.h>
#include "drivers/font.h"
#include "drivers/terminal.h"
#include "drivers/mouse.h"

static uint8_t *fb;
static uint32_t fb_width;
static uint32_t fb_height;
static uint32_t fb_pitch;
static uint32_t fb_bpp;
static uint32_t cursor_x;
static uint32_t cursor_y;



static uint32_t term_fg = TERM_COLOR_FG;
static uint32_t term_bg = TERM_COLOR_BG;



#define CURSOR_SIZE 11
static uint32_t cursor_buffer[CURSOR_SIZE * CURSOR_SIZE];
static int last_mouse_x = -1;
static int last_mouse_y = -1;

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

uint32_t getpixel(uint32_t x, uint32_t y)
{
    if (x >= fb_width || y >= fb_height) return 0;
    uint32_t bytes_per_pixel = fb_bpp / 8;
    uint32_t offset = y * fb_pitch + x * bytes_per_pixel;
    uint8_t *p = fb + offset;
    
    if (fb_bpp == 32)
    {
        return (p[3] << 24) | (p[0] << 16) | (p[1] << 8) | p[2];
    }
    else if (fb_bpp == 24)
    {
        return (p[0] << 16) | (p[1] << 8) | p[2];
    }
    return 0;
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
                putpixel(x + col, y + row, term_fg);
            else
                putpixel(x + col, y + row, term_bg);
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
    if (c == '\b' || c == 127)
    {
        if (cursor_x >= FONT_WIDTH)
        {
            cursor_x -= FONT_WIDTH;
            draw_char(' ', cursor_x, cursor_y);
        }
        else if (cursor_y >= FONT_Y_STEP)
        {
            cursor_y -= FONT_Y_STEP;
            cursor_x = fb_width - FONT_WIDTH;
            draw_char(' ', cursor_x, cursor_y);
        }
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
        for (uint32_t x = 0; x < fb_width; x++)
        {
            putpixel(x, y, TERM_COLOR_BG);
        }
    }

    cursor_x = 0;
    cursor_y = 0;
    last_mouse_x = -1;
    last_mouse_y = -1;
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

static void save_cursor_background(int x, int y)
{
    for (int dy = 0; dy < CURSOR_SIZE; dy++)
    {
        for (int dx = 0; dx < CURSOR_SIZE; dx++)
        {
            cursor_buffer[dy * CURSOR_SIZE + dx] = getpixel(x + dx, y + dy);
        }
    }
}

static void restore_cursor_background(int x, int y)
{
    if (x < 0 || y < 0) return;
    
    for (int dy = 0; dy < CURSOR_SIZE; dy++)
    {
        for (int dx = 0; dx < CURSOR_SIZE; dx++)
        {
            putpixel(x + dx, y + dy, cursor_buffer[dy * CURSOR_SIZE + dx]);
        }
    }
}

static void draw_cursor_shape(int x, int y)
{
    const char cursor_pattern[CURSOR_SIZE][CURSOR_SIZE] = {
        {1,0,0,0,0,0,0,0,0,0,0},
        {1,1,0,0,0,0,0,0,0,0,0},
        {1,2,1,0,0,0,0,0,0,0,0},
        {1,2,2,1,0,0,0,0,0,0,0},
        {1,2,2,2,1,0,0,0,0,0,0},
        {1,2,2,2,2,1,0,0,0,0,0},
        {1,2,2,2,2,2,1,0,0,0,0},
        {1,2,2,2,2,2,2,1,0,0,0},
        {1,2,2,2,2,2,2,2,1,0,0},
        {1,2,2,2,2,1,1,1,1,1,0},
        {1,2,1,0,1,2,1,0,0,0,0},
    };
    
    for (int dy = 0; dy < CURSOR_SIZE; dy++)
    {
        for (int dx = 0; dx < CURSOR_SIZE; dx++)
        {
            uint32_t color = 0;
            switch (cursor_pattern[dy][dx])
            {
                case 1: color = 0xFFFFFF; break;
                case 2: color = 0x000000; break;
                default: continue;
            }
            putpixel(x + dx, y + dy, color);
        }
    }
}

void draw_mouse() 
{
    if (last_mouse_x >= 0 && last_mouse_y >= 0)
    {
        restore_cursor_background(last_mouse_x, last_mouse_y);
    }
    
    save_cursor_background(mouse_x, mouse_y);
    draw_cursor_shape(mouse_x, mouse_y);
    
    last_mouse_x = mouse_x;
    last_mouse_y = mouse_y;
}

void term_set_color(uint32_t fg)
{
    term_fg = fg;
}

void term_reset_color(void)
{
    term_fg = TERM_COLOR_FG;
}
