#include <stdint.h>
#include "terminal.h"
#include "string.h"
#include "syscall.h"

#define VGA ((uint16_t*)0xB8000)
#define W 80
#define H 25
#define COLOR 0x0F

static int cursor_x = 0;
static int cursor_y = 0;

#define LINE_BUF_SIZE 256
static int line_pos = 0;

void idt_set_gate(uint8_t num, uint32_t handler, uint16_t sel, uint8_t flags);

static void scroll() {
    for (int y = 1; y < H; y++)
        for (int x = 0; x < W; x++)
            VGA[(y - 1) * W + x] = VGA[y * W + x];
    for (int x = 0; x < W; x++)
        VGA[(H - 1) * W + x] = (COLOR << 8) | ' ';
    cursor_y = H - 1;
}

void term_putc(char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\b') {
        if (cursor_x > 0) {
            cursor_x--;
            VGA[cursor_y * W + cursor_x] = (COLOR << 8) | ' ';
        }
    } else {
        VGA[cursor_y * W + cursor_x] = (COLOR << 8) | c;
        cursor_x++;
    }
    if (cursor_x >= W) {
        cursor_x = 0;
        cursor_y++;
    }
    if (cursor_y >= H)
        scroll();
}

void term_puts(const char* s) {
    while (*s)
        term_putc(*s++);
}

void term_clear(void) {
    for (int y = 0; y < H; y++)
        for (int x = 0; x < W; x++)
            VGA[y * W + x] = (COLOR << 8) | ' ';
    cursor_x = 0;
    cursor_y = 0;
}

void term_init(void) {
    term_clear();
    line_pos = 0;
}

