#include <stdint.h>
#include "cpu/idt.h"
#include "drivers/terminal.h"
#include "drivers/pic.h"
#include "cpu/gdt.h"
#include "syscall/syscall.h"
#include "user/user.h"
#include "cpu/tss.h"
#include "drivers/ata.h"
#include "drivers/timer.h"
#include "fs/fat32.h"
#include "lib/libc.h"
#include "drivers/rtc.h"
#include "lib/string.h"

static void boot_step(const char *name) {
    term_puts("-> ");
    term_puts(name);

    int len = strlen(name);
    for (int i = 0; i < 30 - len; i++)
        term_putc(' ');
}

static void boot_ok(void) {
    term_puts("[ OK ]\n");
}

static void boot_fail(void) {
    term_puts("[ FAIL ]\n");
    term_puts("kernel panic: fatal error\n");
    asm volatile("cli");
    while (1)
        asm volatile("hlt");
}

void kernel_main(void) {
    term_init();

    term_puts("QuantumKernel booting...\n\n");

    boot_step("gdt");
    gdt_init();
    boot_ok();

    boot_step("tss");
    tss_init();
    boot_ok();

    boot_step("pic");
    pic_remap();
    boot_ok();

    boot_step("idt");
    idt_init();
    boot_ok();

    boot_step("ata");
    ata_init();
    
    ata_error_t err = ata_identify();
    if (err != ATA_OK) {
        term_puts("[FAIL] ");
        term_puts(ata_error_str(err));
        term_putc('\n');
        boot_fail();
    }
    boot_ok();
    

    boot_step("fat32");
    fat32_init();
    fat32_mount();
    boot_ok();

    boot_step("timezone");
    load_timezone();
    boot_ok();

    rtc_time_t t = rtc_get_local_time();
    term_puts("\nTime: ");
    char buf[8];
    itoa(t.hour, buf, 10);
    term_puts(buf);
    term_putc(':');
    itoa(t.min, buf, 10);
    term_puts(buf);
    term_puts("\n");

    boot_step("timer");
    timer_init(100);
    boot_ok();

    boot_step("syscall");
    extern void syscall_handler(void);
    idt_set_gate(0x80, (uint32_t)syscall_handler, 0x08, 0xEE);
    boot_ok();

    boot_step("interrupts");
    asm volatile("sti");
    boot_ok();

    term_puts("\nSystem ready.\n");
    term_puts("Starting init...\n\n");

    enter_user();

    boot_fail();
}
