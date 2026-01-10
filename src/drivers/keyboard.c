#include <stdint.h>
#include "drivers/io.h"

#define KBD_DATA 0x60
#define KBD_BUF_SIZE 128

static char kbd_buf[KBD_BUF_SIZE];
static uint32_t kbd_head = 0;
static uint32_t kbd_tail = 0;

#define KEY_UP    0x80
#define KEY_DOWN  0x81
#define KEY_LEFT  0x82
#define KEY_RIGHT 0x83


static void kbd_push(char c);
int kbd_pop(void);

static char keymap[128] = {
    0,    27,   '1',  '2',  '3',  '4',  '5',  '6',  // 0x00-0x07
    '7',  '8',  '9',  '0',  '-',  '=',  '\b', '\t', // 0x08-0x0F
    'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',  // 0x10-0x17
    'o',  'p',  '[',  ']',  '\n', 0,    'a',  's',  // 0x18-0x1F
    'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';',  // 0x20-0x27
    '\'', '`',  0,    '\\', 'z',  'x',  'c',  'v',  // 0x28-0x2F
    'b',  'n',  'm',  ',',  '.',  '/',  0,    '*',  // 0x30-0x37
    0,    ' ',  
};

static int extended = 0;

void keyboard_irq(void) {
    uint8_t sc = inb(0x60);

    if (sc == 0xE0) {
        extended = 1;
        return;
    }

    if (sc & 0x80) {
        extended = 0;
        return;
    }

    if (extended) {
        switch (sc) {
            case 0x48: kbd_push(KEY_UP); break;
            case 0x50: kbd_push(KEY_DOWN); break;
            case 0x4B: kbd_push(KEY_LEFT); break;
            case 0x4D: kbd_push(KEY_RIGHT); break;
        }
        extended = 0;
        return;
    }

    char c = keymap[sc];
    if (c)
        kbd_push(c);
}


static void kbd_push(char c) {
    uint32_t next = (kbd_head + 1) % KBD_BUF_SIZE;
    if (next != kbd_tail) {
        kbd_buf[kbd_head] = c;
        kbd_head = next;
    }
}

int kbd_pop(void) {
    if (kbd_head == kbd_tail) 
        return -1;
    
    char c = kbd_buf[kbd_tail];
    kbd_tail = (kbd_tail + 1) % KBD_BUF_SIZE;
    return c;
}
