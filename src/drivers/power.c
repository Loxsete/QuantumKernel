#include <stdint.h>
#include "drivers/power.h"
#include "drivers/pci.h"
#include "drivers/io.h"



typedef struct {
    char signature[8];
    uint8_t checksum;
    char oem_id[6];
    uint8_t revision;
    uint32_t rsdt_address;
} __attribute__((packed)) rsdp_t;

typedef struct {
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char oem_id[6];
    char oem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} __attribute__((packed)) acpi_header_t;

typedef struct {
    acpi_header_t header;
    uint32_t tables[];
} __attribute__((packed)) rsdt_t;

typedef struct {
    acpi_header_t header;
    uint32_t firmware_ctrl;
    uint32_t dsdt;
    uint8_t reserved;
    uint8_t preferred_pm_profile;
    uint16_t sci_int;
    uint32_t smi_cmd;
    uint8_t acpi_enable;
    uint8_t acpi_disable;
    uint8_t s4bios_req;
    uint8_t pstate_cnt;
    uint32_t pm1a_evt_blk;
    uint32_t pm1b_evt_blk;
    uint32_t pm1a_cnt_blk;
    uint32_t pm1b_cnt_blk;
    uint32_t pm2_cnt_blk;
    uint32_t pm_tmr_blk;
    uint32_t gpe0_blk;
    uint32_t gpe1_blk;
    uint8_t pm1_evt_len;
    uint8_t pm1_cnt_len;
    uint8_t pm2_cnt_len;
    uint8_t pm_tmr_len;
    uint8_t gpe0_blk_len;
    uint8_t gpe1_blk_len;
    uint8_t gpe1_base;
    uint8_t cst_cnt;
    uint16_t p_lvl2_lat;
    uint16_t p_lvl3_lat;
    uint16_t flush_size;
    uint16_t flush_stride;
    uint8_t duty_offset;
    uint8_t duty_width;
    uint8_t day_alrm;
    uint8_t mon_alrm;
    uint8_t century;
    uint16_t iapc_boot_arch;
    uint8_t reserved2;
    uint32_t flags;
} __attribute__((packed)) fadt_t;

static rsdp_t* rsdp = 0;
static fadt_t* fadt = 0;
static uint16_t pm1a_cnt = 0;
static uint16_t pm1b_cnt = 0;
static uint16_t slp_typa = 0;
static uint16_t slp_typb = 0;
static int acpi_initialized = 0;

static rsdp_t* find_rsdp(void) {
    uint8_t* ptr;
    
    for (ptr = (uint8_t*)0x000E0000; ptr < (uint8_t*)0x00100000; ptr += 16) {
        if (ptr[0] == 'R' && ptr[1] == 'S' && ptr[2] == 'D' && 
            ptr[3] == ' ' && ptr[4] == 'P' && ptr[5] == 'T' && 
            ptr[6] == 'R' && ptr[7] == ' ') {
            uint8_t sum = 0;
            for (int i = 0; i < 20; i++)
                sum += ptr[i];
            if (sum == 0)
                return (rsdp_t*)ptr;
        }
    }
    
    uint16_t* ebda_addr = (uint16_t*)0x040E;
    uint32_t ebda = (*ebda_addr) << 4;
    if (ebda) {
        for (ptr = (uint8_t*)ebda; ptr < (uint8_t*)(ebda + 1024); ptr += 16) {
            if (ptr[0] == 'R' && ptr[1] == 'S' && ptr[2] == 'D' && 
                ptr[3] == ' ' && ptr[4] == 'P' && ptr[5] == 'T' && 
                ptr[6] == 'R' && ptr[7] == ' ') {
                uint8_t sum = 0;
                for (int i = 0; i < 20; i++)
                    sum += ptr[i];
                if (sum == 0)
                    return (rsdp_t*)ptr;
            }
        }
    }
    
    return 0;
}

static fadt_t* find_fadt(rsdt_t* rsdt) {
    int entries = (rsdt->header.length - sizeof(acpi_header_t)) / 4;
    
    for (int i = 0; i < entries; i++) {
        acpi_header_t* h = (acpi_header_t*)rsdt->tables[i];
        if (h->signature[0] == 'F' && h->signature[1] == 'A' && 
            h->signature[2] == 'C' && h->signature[3] == 'P') {
            return (fadt_t*)h;
        }
    }
    return 0;
}

static uint8_t* find_dsdt(fadt_t* fadt) {
    return (uint8_t*)fadt->dsdt;
}

static int parse_s5(uint8_t* dsdt) {
    uint32_t dsdt_len = ((acpi_header_t*)dsdt)->length;
    uint8_t* ptr = dsdt + sizeof(acpi_header_t);
    uint8_t* end = dsdt + dsdt_len;
    
    while (ptr < end - 4) {
        if (ptr[0] == '_' && ptr[1] == 'S' && ptr[2] == '5' && ptr[3] == '_') {
            ptr += 4;
            
            if (*ptr == 0x12) {
                ptr++;
                ptr++;
                if (*ptr == 0x04) {
                    ptr++;
                    if (*ptr == 0x0A) {
                        ptr++;
                        slp_typa = *ptr++;
                    }
                    if (*ptr == 0x0A) {
                        ptr++;
                        slp_typb = *ptr++;
                    }
                    return 1;
                }
            }
        }
        ptr++;
    }
    return 0;
}

void acpi_init(void) {
    rsdp = find_rsdp();
    if (!rsdp) {
        acpi_initialized = 0;
        return;
    }
    
    rsdt_t* rsdt = (rsdt_t*)rsdp->rsdt_address;
    fadt = find_fadt(rsdt);
    if (!fadt) {
        acpi_initialized = 0;
        return;
    }
    
    pm1a_cnt = fadt->pm1a_cnt_blk;
    pm1b_cnt = fadt->pm1b_cnt_blk;
    
    uint8_t* dsdt = find_dsdt(fadt);
    if (dsdt && parse_s5(dsdt)) {
        acpi_initialized = 1;
    } else {
        acpi_initialized = 0;
    }
}

int acpi_is_available(void) {
    return acpi_initialized;
}

void power_reboot(void) {
    uint8_t temp;
    asm volatile("cli");
    
    do {
        temp = inb(0x64);
        if (temp & 0x01)
            inb(0x60);
    } while (temp & 0x02);
    
    outb(0x64, 0xFE);
    
    asm volatile("lidt %0" : : "m"((uint16_t){0}));
    asm volatile("int $0x03");
    
    for(;;) asm volatile("hlt");
}

void power_shutdown(void) {
    asm volatile("cli");
    
    if (acpi_initialized && pm1a_cnt) {
        uint16_t slp_en = 1 << 13;
        outw(pm1a_cnt, slp_en | (slp_typa << 10));
        if (pm1b_cnt)
            outw(pm1b_cnt, slp_en | (slp_typb << 10));
    }
    
    outw(0xB004, 0x2000);
    outw(0x604, 0x2000);
    outw(0x4004, 0x3400);
    
    uint16_t dev = pci_find_device(0x8086, 0x7113);
    if (dev != 0xFFFF) {
        uint32_t pm_base = pci_config_read_dword(0, dev >> 8, dev & 0xFF, 0x40);
        pm_base &= 0xFFC0;
        if (pm_base) {
            outw(pm_base + 4, 0x2000);
        }
    }
    
    for(;;) asm volatile("hlt");
}

void power_halt(void) {
    asm volatile("cli");
    for(;;) asm volatile("hlt");
}

void power_suspend(void) {
    asm volatile("cli");
    asm volatile("hlt");
    asm volatile("sti");
}
