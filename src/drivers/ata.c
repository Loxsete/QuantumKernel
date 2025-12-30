#include "drivers/ata.h"
#include "drivers/io.h"
#include "drivers/terminal.h"

static ata_error_t ata_last_error = ATA_OK;

ata_error_t ata_get_last_error(void) {
    return ata_last_error;
}

static ata_error_t ata_wait_busy(void) {
    int timeout = 100000;
    while (timeout-- > 0) {
        uint8_t status = inb(ATA_PRIMARY_STATUS);
        if (!(status & ATA_SR_BSY))
            return ATA_OK;
    }
    return ATA_ERR_TIMEOUT;
}

static ata_error_t ata_wait_drq(void) {
    int timeout = 100000;
    while (timeout-- > 0) {
        uint8_t status = inb(ATA_PRIMARY_STATUS);

        if (status & ATA_SR_ERR)
            return ATA_ERR_DEVICE;

        if (status & ATA_SR_DRQ)
            return ATA_OK;
    }
    return ATA_ERR_DRQ;
}



void ata_init(void) {
    outb(ATA_PRIMARY_DRIVE, 0xA0);
    ata_wait_busy();
    outb(0x3F6, 0x02);
}


ata_error_t ata_identify(void) {
    outb(ATA_PRIMARY_DRIVE, 0xA0);
    if (ata_wait_busy() != ATA_OK)
        return ATA_ERR_TIMEOUT;

    outb(ATA_PRIMARY_COMMAND, ATA_CMD_IDENTIFY);
    if (ata_wait_busy() != ATA_OK)
        return ATA_ERR_TIMEOUT;

    uint8_t status = inb(ATA_PRIMARY_STATUS);
    if (status == 0)
        return ATA_ERR_NO_DRIVE;

    ata_error_t err = ata_wait_drq();
    if (err != ATA_OK)
        return err;

    for (int i = 0; i < 256; i++)
        (void)inw(ATA_PRIMARY_DATA);

    return ATA_OK;
}


ata_error_t ata_read_sector(uint32_t lba, uint8_t* buffer) {
    if (!buffer)
        return ATA_ERR_BAD_BUFFER;

    if (ata_wait_busy() != ATA_OK)
        return ATA_ERR_TIMEOUT;

    outb(ATA_PRIMARY_DRIVE, 0xE0 | ((lba >> 24) & 0x0F));

    outb(ATA_PRIMARY_SECCOUNT, 1);
    outb(ATA_PRIMARY_LBA_LO,  lba & 0xFF);
    outb(ATA_PRIMARY_LBA_MID, (lba >> 8) & 0xFF);
    outb(ATA_PRIMARY_LBA_HI,  (lba >> 16) & 0xFF);

    outb(ATA_PRIMARY_COMMAND, ATA_CMD_READ_PIO);

    ata_error_t err = ata_wait_drq();
    if (err != ATA_OK)
        return err;

    uint16_t* buf16 = (uint16_t*)buffer;
    for (int i = 0; i < 256; i++)
        buf16[i] = inw(ATA_PRIMARY_DATA);

    return ATA_OK;
}


ata_error_t ata_write_sector(uint32_t lba, const uint8_t* buffer) {
    if (!buffer)
        return ATA_ERR_BAD_BUFFER;

    if (ata_wait_busy() != ATA_OK)
        return ATA_ERR_TIMEOUT;

    outb(ATA_PRIMARY_DRIVE, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_PRIMARY_SECCOUNT, 1);

    outb(ATA_PRIMARY_LBA_LO,  lba & 0xFF);
    outb(ATA_PRIMARY_LBA_MID, (lba >> 8) & 0xFF);
    outb(ATA_PRIMARY_LBA_HI,  (lba >> 16) & 0xFF);

    outb(ATA_PRIMARY_COMMAND, ATA_CMD_WRITE_PIO);

    ata_error_t err = ata_wait_drq();
    if (err != ATA_OK)
        return err;

    const uint16_t* buf16 = (const uint16_t*)buffer;
    for (int i = 0; i < 256; i++)
        outw(ATA_PRIMARY_DATA, buf16[i]);

    outb(ATA_PRIMARY_COMMAND, 0xE7);
    ata_wait_busy();

    return ATA_OK;
}
