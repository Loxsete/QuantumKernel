#include <stdint.h>
#include "io.h"

#define KBD_DATA 0x60
#define KBD_BUF_SIZE 128

static char kbd_buf[KBD_BUF_SIZE];
static uint32_t kbd_head = 0;
static uint32_t kbd_tail = 0;

static void kbd_push(char c);
int kbd_pop(void);

static char keymap[128] = {
    0, 27, '1','2','3','4','5','6','7','8','9','0','-','=', '\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0, 'a','s','d','f','g','h','j','k','l',';','\'','`',
    0, '\\','z','x','c','v','b','n','m',',','.','/',
};

void keyboard_irq(void) {
    uint8_t scancode = inb(KBD_DATA);

    if (scancode & 0x80)
        return;

    char c = keymap[scancode];
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
