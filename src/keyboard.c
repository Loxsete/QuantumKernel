// src/keyboard.c
#include <stdint.h>
#include "terminal.h"
#include "io.h"   // inb

#define KBD_DATA 0x60

static char keymap[128] = {
    0, 27, '1','2','3','4','5','6','7','8','9','0','-','=', '\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0, 'a','s','d','f','g','h','j','k','l',';','\'','`',
    0, '\\','z','x','c','v','b','n','m',',','.','/',
};

void keyboard_irq(void) {
    uint8_t scancode = inb(KBD_DATA);

    if (scancode & 0x80) {
        return;  
    }

    char c = keymap[scancode];
    if (c != 0) {
        term_process_char(c);
    }
}
