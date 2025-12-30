#ifndef ATA_ERROR_H
#define ATA_ERROR_H

typedef enum {
    ATA_OK = 0,

    ATA_ERR_NO_DRIVE      = -1,
    ATA_ERR_TIMEOUT       = -2,
    ATA_ERR_DRQ           = -3,
    ATA_ERR_DEVICE        = -4,
    ATA_ERR_BAD_BUFFER    = -5,
    ATA_ERR_UNKNOWN       = -128
} ata_error_t;

const char* ata_error_str(ata_error_t err);

#endif
