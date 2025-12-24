#include <stdint.h>
#include "idt.h"
#include "terminal.h"
#include "pic.h"
#include "gdt.h"
#include "syscall.h"
#include "user.h"
#include "tss.h"
#include "ata.h" 

void kernel_main(void) {
    /* --- init --- */
    term_init();
    gdt_init();
    tss_init();
    term_puts("Booting kernel...\n");
    pic_remap();
    idt_init();
    
    ata_init();
    ata_identify();
    
    /* syscall */
    extern void syscall_handler(void);
    idt_set_gate(0x80, (uint32_t)syscall_handler, 0x08, 0xEE);
    asm volatile ("sti");
    term_puts("Kernel ready\n> ");
   
    enter_user();
    while (1) {
        asm volatile ("hlt");
    }
}
