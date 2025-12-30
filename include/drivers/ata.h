#ifndef ATA_H
#define ATA_H

#include <stdint.h>
#include "drivers/errors/ata_error.h"

/* ATA ports */
#define ATA_PRIMARY_DATA       0x1F0
#define ATA_PRIMARY_ERR        0x1F1
#define ATA_PRIMARY_SECCOUNT   0x1F2
#define ATA_PRIMARY_LBA_LO     0x1F3
#define ATA_PRIMARY_LBA_MID    0x1F4
#define ATA_PRIMARY_LBA_HI     0x1F5
#define ATA_PRIMARY_DRIVE      0x1F6
#define ATA_PRIMARY_COMMAND    0x1F7
#define ATA_PRIMARY_STATUS     0x1F7

/* ATA commands */
#define ATA_CMD_READ_PIO       0x20
#define ATA_CMD_WRITE_PIO      0x30
#define ATA_CMD_IDENTIFY       0xEC

/* Status bits */
#define ATA_SR_BSY   0x80
#define ATA_SR_DRDY  0x40
#define ATA_SR_DRQ   0x08
#define ATA_SR_ERR   0x01

void ata_init(void);
ata_error_t ata_identify(void);
ata_error_t ata_read_sector(uint32_t lba, uint8_t* buffer);
ata_error_t ata_write_sector(uint32_t lba, const uint8_t* buffer);

#endif
