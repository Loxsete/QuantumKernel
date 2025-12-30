#include "drivers/rtl8139.h"
#include "drivers/terminal.h"
#include "lib/libc.h"
#include "drivers/io.h"
#include "cpu/idt.h"

#define REG_MAC         0x00
#define REG_MAR         0x08
#define REG_TXSTATUS0   0x10
#define REG_TXADDR0     0x20
#define REG_RXBUF       0x30
#define REG_COMMAND     0x37
#define REG_CAPR        0x38
#define REG_IMR         0x3C
#define REG_ISR         0x3E
#define REG_RXCONFIG    0x44
#define REG_TXCONFIG    0x40
#define REG_CONFIG1     0x52

#define CMD_RESET       0x10
#define CMD_RX_ENABLE   0x08
#define CMD_TX_ENABLE   0x04

#define ISR_ROK         0x0001
#define ISR_RXERR       0x0002
#define ISR_TOK         0x0004
#define ISR_TXERR       0x0008

#define RXCFG_ACCEPT_ALL    0x0F
#define RXCFG_WRAP          0x80
#define RXCFG_BUFFER_SIZE   (8192 << 11)

static uint32_t io_base;
static uint8_t irq_line;
static uint8_t rx_buffer[8192 + 16 + 1500] __attribute__((aligned(4096)));
static uint16_t current_rx_offset = 0;

uint8_t mac_addr[6];

extern void irq10_handler();

void rtl8139_init(pci_device_t *dev) {
    io_base = dev->bar[0] & ~3;
    irq_line = dev->interrupt_line;

    uint16_t command = pci_config_read_word(dev->bus, dev->device, dev->function, 0x04);
    pci_config_write_word(dev->bus, dev->device, dev->function, 0x04, command | (1 << 2));

    outb(io_base + REG_CONFIG1, 0x00);

    outb(io_base + REG_COMMAND, CMD_RESET);
    while (inb(io_base + REG_COMMAND) & CMD_RESET) {}

    for (int i = 0; i < 6; i++) {
        mac_addr[i] = inb(io_base + REG_MAC + i);
    }

    outl(io_base + REG_RXBUF, (uint32_t)rx_buffer);

    outw(io_base + REG_IMR, ISR_ROK | ISR_TOK | ISR_RXERR | ISR_TXERR);

    outl(io_base + REG_RXCONFIG, RXCFG_ACCEPT_ALL | RXCFG_WRAP | RXCFG_BUFFER_SIZE);

    idt_set_gate(32 + irq_line, (uint32_t)irq10_handler, 0x08, 0x8E);

    outb(io_base + REG_COMMAND, CMD_RX_ENABLE | CMD_TX_ENABLE);

    term_puts("RTL8139 initialized, MAC: ");
    for (int i = 0; i < 6; i++) {
        char hex[3];
        itoa(mac_addr[i], hex, 16);
        if (mac_addr[i] < 0x10) term_putc('0');
        term_puts(hex);
        if (i < 5) term_putc(':');
    }
    term_putc('\n');
}

void rtl8139_irq_handler(void) {
    uint16_t status = inw(io_base + REG_ISR);
    outw(io_base + REG_ISR, status);

    if (status & (ISR_ROK | ISR_RXERR)) {
        while ((inb(io_base + REG_COMMAND) & 0x01) == 0) {
            uint32_t offset = current_rx_offset % 8192;
            uint32_t rx_status = *(uint32_t*)(rx_buffer + offset);
            uint16_t packet_len = rx_status >> 16;

            current_rx_offset = (current_rx_offset + packet_len + 4 + 3) & ~3;
            outw(io_base + REG_CAPR, current_rx_offset - 0x10);
        }
    }

    if (status & (ISR_TOK | ISR_TXERR)) {
        term_puts("RTL8139: Transmit complete\n");
    }
}
