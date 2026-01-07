#include "drivers/rtl8139.h"
#include "drivers/terminal.h"
#include "drivers/io.h"
#include "cpu/idt.h"
#include "drivers/net/ethernet.h"
#include "lib/libc.h"
#include "mm/kmalloc.h"

#define REG_MAC         0x00
#define REG_TXSTATUS0   0x10
#define REG_TXADDR0     0x20
#define REG_RXBUF       0x30
#define REG_COMMAND     0x37
#define REG_CAPR        0x38
#define REG_IMR         0x3C
#define REG_ISR         0x3E
#define REG_TXCONFIG    0x40
#define REG_RXCONFIG    0x44
#define REG_CONFIG1     0x52

#define CMD_RESET       0x10
#define CMD_RX_ENABLE   0x08
#define CMD_TX_ENABLE   0x04

#define ISR_ROK         0x0001
#define ISR_RXERR       0x0002
#define ISR_TOK         0x0004
#define ISR_TXERR       0x0008

static uint32_t io_base = 0;
static uint8_t irq_line;
static uint32_t rx_buffer_phys;
static uint32_t tx_buffer_phys[4];
static uint16_t current_rx_offset;
uint8_t mac_addr[6];

extern void irq10_handler();

void rtl8139_send(void *data, uint16_t len) {
    if (!io_base) {
        term_puts("[RTL8139] ERROR: Not initialized\n");
        return;
    }
    
    static int tx = 0;
    
    term_puts("[RTL8139] TX: len=");
    char buf[16];
    itoa(len, buf, 10);
    term_puts(buf);
    term_puts(", desc=");
    itoa(tx, buf, 10);
    term_puts(buf);
    term_puts("\n");
    
    if (len > 1792) {
        term_puts("[RTL8139] ERROR: Packet too large\n");
        return;
    }
    
    memcpy((void*)tx_buffer_phys[tx], data, len);
    
    uint32_t status = len & 0x1FFF;
    outl(io_base + REG_TXSTATUS0 + tx * 4, status);
    
    tx = (tx + 1) & 3;
}

void rtl8139_init(pci_device_t *dev) {
    term_puts("[RTL8139] Initializing...\n");
    
    io_base = dev->bar[0] & ~3;
    irq_line = dev->interrupt_line;
    
    term_puts("[RTL8139] IO base: ");
    char buf[16];
    itoa(io_base, buf, 16);
    term_puts(buf);
    term_puts(", IRQ: ");
    itoa(irq_line, buf, 10);
    term_puts(buf);
    term_puts("\n");
    
    uint16_t cmd = pci_config_read_word(dev->bus, dev->device, dev->function, 0x04);
    pci_config_write_word(dev->bus, dev->device, dev->function, 0x04, cmd | 0x07);
    term_puts("[RTL8139] PCI command set\n");
    
    outb(io_base + REG_CONFIG1, 0x00);
    term_puts("[RTL8139] Power on\n");
    
    outb(io_base + REG_COMMAND, CMD_RESET);
    while (inb(io_base + REG_COMMAND) & CMD_RESET) {}
    term_puts("[RTL8139] Reset complete\n");
    
    rx_buffer_phys = (uint32_t)kmalloc_a(8192 + 16 + 1500);
    term_puts("[RTL8139] RX buffer: ");
    itoa(rx_buffer_phys, buf, 16);
    term_puts(buf);
    term_puts("\n");
    
    for (int i = 0; i < 4; i++) {
        tx_buffer_phys[i] = (uint32_t)kmalloc_a(1792);
        outl(io_base + REG_TXADDR0 + i * 4, tx_buffer_phys[i]);
        term_puts("[RTL8139] TX buffer ");
        itoa(i, buf, 10);
        term_puts(buf);
        term_puts(": ");
        itoa(tx_buffer_phys[i], buf, 16);
        term_puts(buf);
        term_puts("\n");
    }
    
    for (int i = 0; i < 6; i++) {
        mac_addr[i] = inb(io_base + REG_MAC + i);
    }
    
    outl(io_base + REG_RXBUF, rx_buffer_phys);
    
    outw(io_base + REG_IMR, ISR_ROK | ISR_TOK | ISR_RXERR | ISR_TXERR);
    
    outl(io_base + REG_RXCONFIG, 0x0000000F);
    
    outl(io_base + REG_TXCONFIG, 0x03000000);
    
    current_rx_offset = 0;
    outw(io_base + REG_CAPR, 0xFFF0);
    
    idt_set_gate(32 + irq_line, (uint32_t)irq10_handler, 0x08, 0x8E);
    term_puts("[RTL8139] IRQ handler installed at vector ");
    itoa(32 + irq_line, buf, 10);
    term_puts(buf);
    term_puts("\n");
    
    outb(io_base + REG_COMMAND, CMD_RX_ENABLE | CMD_TX_ENABLE);
    
    eth_init(mac_addr);
    
    term_puts("[RTL8139] MAC: ");
    for (int i = 0; i < 6; i++) {
        itoa(mac_addr[i], buf, 16);
        if (mac_addr[i] < 16) term_putc('0');
        term_puts(buf);
        if (i < 5) term_putc(':');
    }
    term_puts("\n");
    
    term_puts("[RTL8139] Initialization complete\n");
}

void rtl8139_irq_handler(void) {
    if (!io_base) return;
    
    uint16_t status = inw(io_base + REG_ISR);
    
    term_puts("[RTL8139] IRQ: status=");
    char buf[16];
    itoa(status, buf, 16);
    term_puts(buf);
    term_puts("\n");
    
    if (status & ISR_ROK) {
        term_puts("[RTL8139] RX packet(s)\n");
        
        while (!(inb(io_base + REG_COMMAND) & 0x01)) {
            uint32_t off = current_rx_offset;
            
            if (off >= 8192) {
                term_puts("[RTL8139] RX overflow, resetting\n");
                current_rx_offset = 0;
                outw(io_base + REG_CAPR, 0xFFF0);
                break;
            }
            
            uint32_t *hdr_ptr = (uint32_t*)(rx_buffer_phys + off);
            uint32_t hdr = *hdr_ptr;
            uint16_t pkt_status = hdr & 0xFFFF;
            uint16_t pkt_len = (hdr >> 16) & 0xFFFF;
            
            term_puts("[RTL8139] RX: len=");
            itoa(pkt_len, buf, 10);
            term_puts(buf);
            term_puts(", status=");
            itoa(pkt_status, buf, 16);
            term_puts(buf);
            term_puts("\n");
            
            if ((pkt_status & 0x01) && pkt_len >= 60 && pkt_len <= 1518) {
                uint8_t *pkt = (uint8_t*)(rx_buffer_phys + off + 4);
                eth_rx(pkt, pkt_len - 4);
            } else {
                term_puts("[RTL8139] Bad packet\n");
            }
            
            current_rx_offset = (off + pkt_len + 4 + 3) & ~3;
            outw(io_base + REG_CAPR, current_rx_offset - 16);
        }
    }
    
    if (status & ISR_TOK) {
        term_puts("[RTL8139] TX complete\n");
    }
    
    if (status & ISR_TXERR) {
        term_puts("[RTL8139] TX error\n");
    }
    
    if (status & ISR_RXERR) {
        term_puts("[RTL8139] RX error\n");
    }
    
    outw(io_base + REG_ISR, status);
}
