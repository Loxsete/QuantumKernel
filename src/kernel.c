#include <stdint.h>
#include "idt.h"
#include "terminal.h"
#include "pic.h"

void kernel_main(void) {
    /* --- init --- */
    term_init();
    
    term_puts("Booting kernel...\n");

    pic_remap();
    idt_init();

    /* syscall */
    extern void syscall_handler(void);
    idt_set_gate(0x80, (uint32_t)syscall_handler, 0x08, 0xEE);

    
    asm volatile ("sti");

    term_puts("Kernel ready\n> ");

    /* test syscall PUTS */
    asm volatile (
        "mov $1, %%eax\n"      /* SYS_PUTS */
        "mov %0, %%ebx\n"
        "int $0x80\n"
        :
        : "r"("Hello from syscall!\n")
        : "eax", "ebx"
    );

    
    while (1) {
        asm volatile ("hlt");
    }
}
