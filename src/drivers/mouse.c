#include <stdint.h>
#include "drivers/pic.h"

#define PS2_DATA   0x60
#define PS2_STATUS 0x64
#define PS2_CMD    0x64

static inline uint8_t inb(uint16_t port) {
    uint8_t r;
    asm volatile ("inb %1, %0" : "=a"(r) : "Nd"(port));
    return r;
}

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline void ps2_wait_read() {
    while (!(inb(PS2_STATUS) & 1));
}

static inline void ps2_wait_write() {
    while (inb(PS2_STATUS) & 2);
}

static inline void ps2_write(uint8_t val) {
    ps2_wait_write();
    outb(PS2_DATA, val);
}

static inline uint8_t ps2_read() {
    ps2_wait_read();
    return inb(PS2_DATA);
}

int mouse_x = 100;
int mouse_y = 100;
int mouse_buttons = 0;

static uint8_t mouse_cycle = 0;
static int8_t mouse_packet[3];

void mouse_init() {
    outb(PS2_CMD, 0xA8);
    ps2_wait_write();
    outb(PS2_CMD, 0x20);
    uint8_t status = ps2_read();
    status |= 0x02;
    ps2_wait_write();
    outb(PS2_CMD, 0x60);
    ps2_write(status);
    ps2_wait_write();
    outb(PS2_CMD, 0xD4);
    ps2_write(0xF4);
    ps2_read();
    pic_enable_irq(12);
}

void mouse_irq() {
    uint8_t data = inb(PS2_DATA);

    if (mouse_cycle == 0) {
        if (!(data & 0x08)) return;
        mouse_packet[0] = data;
        mouse_cycle = 1;
        return;
    }

    if (mouse_cycle == 1) {
        mouse_packet[1] = data;
        mouse_cycle = 2;
        return;
    }

    mouse_packet[2] = data;
    mouse_cycle = 0;

    mouse_buttons = mouse_packet[0] & 7;

    mouse_x += mouse_packet[1];
    mouse_y -= mouse_packet[2];

    if (mouse_x < 0) mouse_x = 0;
    if (mouse_y < 0) mouse_y = 0;
    if (mouse_x > 1919) mouse_x = 1919;
    if (mouse_y > 1079) mouse_y = 1079;
}
