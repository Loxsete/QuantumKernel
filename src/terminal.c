#include <stdint.h>
#include "terminal.h"
#include "string.h"

#define VGA ((uint16_t*)0xB8000)
#define W 80
#define H 25
#define COLOR 0x0F

static int cursor_x = 0;
static int cursor_y = 0;

#define LINE_BUF_SIZE 256
static char line_buffer[LINE_BUF_SIZE];
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
    term_puts("Simple OS Terminal v0.1\n");
    term_puts("> ");
    line_pos = 0;
}

void term_process_char(char c) {
    if (c == 0) return;

    if (c == '\n' || c == '\r') {
        term_putc('\n');
        line_buffer[line_pos] = '\0';

        if (line_pos > 0) {
            if (strcmp(line_buffer, "clear") == 0) {
                term_clear();
                term_puts("> ");
            } else if (strcmp(line_buffer, "help") == 0) {
                term_puts("Available commands:\n");
                term_puts(" help - show this help\n");
                term_puts(" clear - clear screen\n");
                term_puts(" syscall - test syscall\n");
                term_puts("> ");
            } else if (strcmp(line_buffer, "syscall") == 0) {
                extern void syscall_handler(void);  
                idt_set_gate(0x80, (uint32_t)syscall_handler, 0x08, 0xEE);

                term_puts("Syscall handler installed\n> ");

                asm volatile (
                    "mov $1, %%eax\n"      // SYS_PUTS
                    "mov %0, %%ebx\n"
                    "int $0x80\n"
                    :
                    : "r"("Hello from syscall!\n")
                    : "eax", "ebx"
                );
            } else {
                term_puts("Unknown command: ");
                term_puts(line_buffer);
                term_puts("\n> ");
            }
        } else {
            term_puts("> ");
        }
        line_pos = 0;
    } else if (c == '\b' || c == 127) {
        if (line_pos > 0) {
            line_pos--;
            term_putc('\b');
        }
    } else if (c >= 32 && c <= 126 && line_pos < LINE_BUF_SIZE - 1) {
        line_buffer[line_pos++] = c;
        term_putc(c);
    }
}
