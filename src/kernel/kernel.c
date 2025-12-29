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

static void boot_msg(const char *msg) {
    term_puts("[boot] ");
    term_puts(msg);
}

static void boot_ok(void) {
    term_puts(" ... ok\n");
}

extern void fat32_test(void);

void kernel_main(void) {
    term_init();
    
    
    boot_msg("gdt");
    gdt_init();
    boot_ok();
    
    boot_msg("tss");
    tss_init();
    boot_ok();
    
    boot_msg("pic");
    pic_remap();
    boot_ok();
    
    boot_msg("idt");
    idt_init();
    boot_ok();
    
    boot_msg("ata");
    ata_init();
    ata_identify();
    boot_ok();

	boot_msg("fat32");
	fat32_init();
	fat32_mount();
	boot_ok();

	boot_msg("timezone");
	load_timezone();
	boot_ok();
	
	rtc_time_t t = rtc_get_local_time();
	term_puts("Current local time: ");
	char buf[32];
	itoa(t.hour, buf, 10);
	term_puts(buf);
	term_puts(":");
	itoa(t.min, buf, 10);
	term_puts(buf);
	term_puts("\n");
	
    
    boot_msg("timer");
    timer_init(100);
    boot_ok();
    
    boot_msg("syscall");
    extern void syscall_handler(void);
    idt_set_gate(0x80, (uint32_t)syscall_handler, 0x08, 0xEE);
    boot_ok();
    
    boot_msg("interrupts");
    asm volatile ("sti");
    boot_ok();
    
    
    
    term_puts("\nSystem ready\n");
    
    
    term_puts("\n\nStarting init process\n\n");
    enter_user();
    
    term_puts("panic: returned from user mode\n");
    while (1) {
        asm volatile ("hlt");
    }
}
