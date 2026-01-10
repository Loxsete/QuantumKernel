#include <stdint.h>
#include "drivers/power.h"
#include "drivers/terminal.h"
#include "kernel/multiboot.h"
#include "cpu/idt.h"
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
#include "drivers/pci.h"
#include "drivers/net/ethernet.h"
#include "drivers/net/arp.h"
#include "drivers/net/ip.h"
#include "drivers/rtl8139.h"
#include "drivers/mouse.h"


static void boot_step(const char *name)
{
    term_puts("-> ");
    term_puts(name);
    int len = strlen(name);
    for (int i = 0; i < 30 - len; i++)
        term_putc(' ');
}

static void boot_ok(void)
{
    term_puts("[ OK ]\n");
}

static void boot_fail(void)
{
    term_puts("[ FAIL ]\n");
    term_puts("kernel panic: fatal error\n");
    asm volatile("cli");
    for (;;)
        asm volatile("hlt");
}

void kernel_main(uint32_t magic, uint32_t mb_addr)
{
    if (magic != 0x2BADB002)
        boot_fail();
    
    multiboot_info_t *mbi = (multiboot_info_t*)mb_addr;
    int fb_ok = 0;
    
    if (
        (mbi->flags & (1 << 12)) &&
        mbi->framebuffer_type == 1 &&
        (mbi->framebuffer_bpp == 24 || mbi->framebuffer_bpp == 32) &&
        mbi->framebuffer_addr != 0 &&
        mbi->framebuffer_pitch != 0
    )
    {
        term_init_fb(
            (uint32_t)mbi->framebuffer_addr,
            mbi->framebuffer_width,
            mbi->framebuffer_height,
            mbi->framebuffer_pitch,
            mbi->framebuffer_bpp
        );
        fb_ok = 1;
    }
    
    if (!fb_ok)
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
    if (err != ATA_OK)
    {
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

    boot_step("pci");
    pci_enumerate();
    pci_register_drivers();
    boot_ok();

	boot_step("acpi");
    acpi_init();
    if (acpi_is_available())
        boot_ok();
    else {
        term_puts("[ WARN ]\n");
    }

    boot_step("net");
    eth_init(mac_addr);
    arp_init(0x0A01A8C0);
    ip_init(0x0A01A8C0);
    boot_ok();

    boot_step("mouse");
    mouse_init();
    boot_ok();
    

    boot_step("interrupts");
    asm volatile("sti");
    boot_ok();

    term_puts("\nSystem ready.\n");
    term_puts("Starting init...\n\n");
    enter_user();

    boot_fail();
}
