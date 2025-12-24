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
#include "lib/libc.h"

void kernel_main(void) {
    term_init();
    term_puts("Step 1: Terminal initialized\n");
    
    term_puts("Step 2: Initializing GDT...\n");
    gdt_init();
    term_puts("GDT OK\n");
    
    term_puts("Step 3: Initializing TSS...\n");
    tss_init();
    term_puts("TSS OK\n");
    
    term_puts("Step 4: Initializing PIC...\n");
    pic_remap();
    term_puts("PIC OK\n");
    
    term_puts("Step 5: Initializing IDT...\n");
    idt_init();
    term_puts("IDT OK\n");
    
    term_puts("Step 6: Initializing ATA...\n");
    ata_init();
    term_puts("ATA init OK\n");
    
    ata_identify();
    term_puts("ATA identify OK\n");
    
    term_puts("Step 7: Initializing Timer...\n");
    timer_init(100);
    term_puts("Timer OK\n");
    
    term_puts("Step 8: Setting up syscall...\n");
    extern void syscall_handler(void);
    idt_set_gate(0x80, (uint32_t)syscall_handler, 0x08, 0xEE);
    term_puts("Syscall OK\n");
    
    term_puts("Step 9: Enabling interrupts...\n");
    asm volatile ("sti");
    term_puts("Interrupts enabled\n");
    
    term_puts("Step 10: Testing timer...\n");
    uint32_t test_start = get_tick_count();
    char buf[16];
    itoa(test_start, buf, 10);
    term_puts("Start ticks: ");
    term_puts(buf);
    term_puts("\n");
    
    for (volatile int i = 0; i < 50000000; i++);
    
    uint32_t test_end = get_tick_count();
    itoa(test_end, buf, 10);
    term_puts("End ticks: ");
    term_puts(buf);
    term_puts("\n");
    
    if (test_end > test_start) {
        term_puts("Timer working!\n");
    } else {
        term_puts("Timer NOT working!\n");
    }
    
    term_puts("Step 11: Entering user mode...\n");
    term_puts("If system reboots here, problem is in enter_user()\n");
    
    for (volatile int i = 0; i < 10000000; i++);
    
    enter_user();
    
    term_puts("ERROR: Returned from user mode!\n");
    while (1) {
        asm volatile ("hlt");
    }
}
