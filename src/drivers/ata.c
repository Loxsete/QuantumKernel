#include "drivers/ata.h"
#include "drivers/io.h"
#include "drivers/terminal.h"

static void ata_wait_busy(void) {
    uint8_t status;
    int timeout = 100000;
    while (timeout-- > 0) {
        status = inb(ATA_PRIMARY_STATUS);
        if (!(status & ATA_SR_BSY))
            return;
    }
    term_puts("[ATA] Timeout waiting for busy\n");
}

static int ata_wait_drq(void) {
    uint8_t status;
    int timeout = 100000;
    while (timeout-- > 0) {
        status = inb(ATA_PRIMARY_STATUS);
        if (status & ATA_SR_DRQ)
            return 0;
        if (status & ATA_SR_ERR)
            return -1;
    }
    term_puts("[ATA] Timeout waiting for DRQ\n");
    return -1;
}

void ata_init(void) {
    term_puts("Initializing ATA driver...\n");
    
    
    outb(ATA_PRIMARY_DRIVE, 0xA0);
    ata_wait_busy();
    
    
    outb(0x3F6, 0x02);
    
    term_puts("ATA driver initialized\n");
}

void ata_identify(void) {
    term_puts("Identifying ATA drive...\n");
    
    
    outb(ATA_PRIMARY_DRIVE, 0xA0);
    ata_wait_busy();
    
    
    outb(ATA_PRIMARY_COMMAND, ATA_CMD_IDENTIFY);
    ata_wait_busy();
    
    uint8_t status = inb(ATA_PRIMARY_STATUS);
    if (status == 0) {
        term_puts("No drive detected\n");
        return;
    }
    
    if (ata_wait_drq() != 0) {
        term_puts("Drive error during identify\n");
        return;
    }
    
    uint16_t identify[256];
    for (int i = 0; i < 256; i++) {
        identify[i] = inw(ATA_PRIMARY_DATA);
    }
    
    term_puts("ATA drive detected\n");
    
    
    term_puts("Model: ");
    for (int i = 27; i < 47; i++) {
        char c1 = (identify[i] >> 8) & 0xFF;
        char c2 = identify[i] & 0xFF;
        if (c1 && c1 != ' ') term_putc(c1);
        if (c2 && c2 != ' ') term_putc(c2);
    }
    term_putc('\n');
}

int ata_read_sector(uint32_t lba, uint8_t* buffer) {
    if (!buffer) return -1;
    
    ata_wait_busy();
    
    
    outb(ATA_PRIMARY_DRIVE, 0xE0 | ((lba >> 24) & 0x0F));
    ata_wait_busy();
    
    
    outb(ATA_PRIMARY_SECCOUNT, 1);
    
    
    outb(ATA_PRIMARY_LBA_LO,  lba & 0xFF);
    outb(ATA_PRIMARY_LBA_MID, (lba >> 8) & 0xFF);
    outb(ATA_PRIMARY_LBA_HI,  (lba >> 16) & 0xFF);
    
    
    outb(ATA_PRIMARY_COMMAND, ATA_CMD_READ_PIO);
    
    ata_wait_busy();
    
    
    uint8_t status = inb(ATA_PRIMARY_STATUS);
    if (status & ATA_SR_ERR) {
        term_puts("[ATA] Read error\n");
        return -1;
    }
    
    if (ata_wait_drq() != 0) {
        term_puts("[ATA] DRQ timeout\n");
        return -1;
    }
    
    
    uint16_t* buf16 = (uint16_t*)buffer;
    for (int i = 0; i < 256; i++) {
        buf16[i] = inw(ATA_PRIMARY_DATA);
    }
    
    return 0;
}

int ata_write_sector(uint32_t lba, const uint8_t* buffer) {
    if (!buffer) return -1;
    
    ata_wait_busy();
    
    
    outb(ATA_PRIMARY_DRIVE, 0xE0 | ((lba >> 24) & 0x0F));
    ata_wait_busy();
    
    
    outb(ATA_PRIMARY_SECCOUNT, 1);
    
    
    outb(ATA_PRIMARY_LBA_LO,  lba & 0xFF);
    outb(ATA_PRIMARY_LBA_MID, (lba >> 8) & 0xFF);
    outb(ATA_PRIMARY_LBA_HI,  (lba >> 16) & 0xFF);
    
    
    outb(ATA_PRIMARY_COMMAND, ATA_CMD_WRITE_PIO);
    
    ata_wait_busy();
    
    if (ata_wait_drq() != 0) {
        term_puts("[ATA] Write DRQ timeout\n");
        return -1;
    }
    
    
    const uint16_t* buf16 = (const uint16_t*)buffer;
    for (int i = 0; i < 256; i++) {
        outw(ATA_PRIMARY_DATA, buf16[i]);
    }
    
    
    outb(ATA_PRIMARY_COMMAND, 0xE7);
    ata_wait_busy();
    
    return 0;
}
