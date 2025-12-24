#include <stdint.h>
#include "drivers/io.h"

#define KBD_DATA 0x60
#define KBD_BUF_SIZE 128

static char kbd_buf[KBD_BUF_SIZE];
static uint32_t kbd_head = 0;
static uint32_t kbd_tail = 0;

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

void keyboard_irq(void) {
    uint8_t scancode = inb(KBD_DATA);
    
    if (scancode & 0x80) 
        return;
    
    if (scancode < 128) {
        char c = keymap[scancode];
        if (c) 
            kbd_push(c);
    }
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
