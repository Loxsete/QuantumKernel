#include <stdint.h>
#include "terminal.h"
#include "keyboard.h"
#include "syscall.h"

typedef struct regs {
    uint32_t gs, fs, es, ds;
    uint32_t edi, esi, ebp, esp;
    uint32_t ebx, edx, ecx, eax;
} regs_t;

void syscall_dispatch(regs_t* r) {
    switch (r->eax) {

        case SYS_PUTS:
            term_puts((const char*)r->ebx);
            break;

        case SYS_PUTC:
            term_putc((char)r->ebx);
            break;

        case SYS_GETCHAR:
            r->eax = 0;
            break;

        case SYS_CLEAR:
            term_clear();
            break;

        case SYS_CURSOR:
            // TODO !!
            break;

        case SYS_EXIT:
            term_puts("\n[process exited]\n");
            while (1) asm volatile("hlt");
            break;

        default:
            term_puts("[unknown syscall]\n");
            break;
    }
}
