#include <stdint.h>
#include "drivers/power.h"
#include "drivers/io.h"
#include "drivers/terminal.h"


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
} __attribute__((packed)) acpi_header_t;

typedef struct {
    acpi_header_t header;
    uint32_t entries[];
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
    uint8_t reserved2[10];
    uint32_t pm1a_cnt_blk;
    uint32_t pm1b_cnt_blk;
} __attribute__((packed)) fadt_t;

static uint16_t pm1a_cnt = 0;
static uint16_t pm1b_cnt = 0;
static uint16_t slp_typa = 0;
static uint16_t slp_typb = 0;
static int acpi_ok = 0;
static int acpi_initialized;




static uint8_t checksum(void* ptr, uint32_t len) {
    uint8_t sum = 0;
    uint8_t* p = ptr;
    while (len--) sum += *p++;
    return sum;
}

static rsdp_t* find_rsdp(void) {
    for (uint8_t* p = (uint8_t*)0xE0000; p < (uint8_t*)0x100000; p += 16) {
        if (!checksum(p, 20) &&
            p[0]=='R'&&p[1]=='S'&&p[2]=='D'&&p[3]==' ' &&
            p[4]=='P'&&p[5]=='T'&&p[6]=='R'&&p[7]==' ') {
            return (rsdp_t*)p;
        }
    }
    return 0;
}

static fadt_t* find_fadt(rsdt_t* rsdt) {
    int count = (rsdt->header.length - sizeof(acpi_header_t)) / 4;
    for (int i = 0; i < count; i++) {
        acpi_header_t* h = (acpi_header_t*)rsdt->entries[i];
        if (h->signature[0]=='F'&&h->signature[1]=='A'&&
            h->signature[2]=='C'&&h->signature[3]=='P')
            return (fadt_t*)h;
    }
    return 0;
}

static int parse_s5(uint8_t* dsdt) {
    acpi_header_t* h = (acpi_header_t*)dsdt;
    uint8_t* p = dsdt + sizeof(acpi_header_t);
    uint8_t* end = dsdt + h->length;

    while (p + 4 < end) {
        if (p[0]=='_'&&p[1]=='S'&&p[2]=='5'&&p[3]=='_') {
            p += 4;
            p += 2; 
            if (*p++ == 0x0A) slp_typa = *p++;
            if (*p++ == 0x0A) slp_typb = *p++;
            return 1;
        }
        p++;
    }
    return 0;
}


void acpi_init(void) {
    rsdp_t* rsdp = find_rsdp();
    if (!rsdp) return;

    rsdt_t* rsdt = (rsdt_t*)rsdp->rsdt_address;
    fadt_t* fadt = find_fadt(rsdt);
    if (!fadt) return;

    pm1a_cnt = fadt->pm1a_cnt_blk;
    pm1b_cnt = fadt->pm1b_cnt_blk;

    if (!pm1a_cnt) return;

    if (parse_s5((uint8_t*)fadt->dsdt))
        acpi_ok = 1;
}

int acpi_is_available(void) {
    return acpi_ok;
}


void power_shutdown(void) {
    term_puts("[power] shutdown requested\n");

    if (acpi_initialized && pm1a_cnt && slp_typa) {
        term_puts("[power] trying ACPI S5\n");

        uint16_t slp_en = 1 << 13;
        outw(pm1a_cnt, (slp_typa << 10) | slp_en);

        if (pm1b_cnt)
            outw(pm1b_cnt, (slp_typb << 10) | slp_en);

        for (volatile int i = 0; i < 10000000; i++);
    }

    term_puts("[power] trying QEMU/Bochs ports\n");
    outw(0x604, 0x2000);
    outw(0xB004, 0x2000);
    outw(0x4004, 0x3400);

    for (volatile int i = 0; i < 10000000; i++);

    term_puts("[power] shutdown failed, halting CPU\n");
    asm volatile("cli");
    for (;;)
        asm volatile("hlt");
}


void power_reboot(void) {
    asm volatile("cli");

    while (inb(0x64) & 0x02);
    outb(0x64, 0xFE);

    asm volatile("lidt (%0)" :: "r"(0));
    asm volatile("int $0x03");

    for (;;)
        asm volatile("hlt");
}

void power_halt(void) {
    asm volatile("cli");
    for (;;)
        asm volatile("hlt");
}
