#include "drivers/errors/ata_error.h"

const char* ata_error_str(ata_error_t err) {
    switch (err) {
        case ATA_OK:             return "ok";
        case ATA_ERR_NO_DRIVE:   return "no drive detected";
        case ATA_ERR_TIMEOUT:    return "timeout";
        case ATA_ERR_DRQ:        return "DRQ error";
        case ATA_ERR_DEVICE:     return "device error";
        case ATA_ERR_BAD_BUFFER: return "bad buffer";
        default:                 return "unknown ATA error";
    }
}
