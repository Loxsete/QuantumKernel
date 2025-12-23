#include <stdint.h>
#include "terminal.h"
#include "keyboard.h"
#include "syscall.h"
#include "syscall_raw.h"

typedef struct regs {
    uint32_t gs, fs, es, ds;
    uint32_t edi, esi, ebp, esp;
    uint32_t ebx, edx, ecx, eax;
} regs_t;

void syscall_dispatch(regs_t* r) {
    switch (r->eax) {

        case SYS_WRITE:
            if (r->ebx == 1) { // stdout
                const char* s = (const char*)r->ecx;
                for (uint32_t i = 0; i < r->edx; i++)
                    term_putc(s[i]);
            }
            r->eax = r->edx; // bytes written
            break;

        case SYS_EXIT:
            term_puts("\n[process exited]\n");
            for (;;)
                asm volatile("hlt");
            break;

        default:
            term_puts("[unknown syscall]\n");
            break;
    }
}


int write(int fd, const char* buf, uint32_t len) {
    return syscall_invoke(SYS_WRITE, fd, (int)buf, len);
}

void exit(void) {
    syscall_invoke(SYS_EXIT, 0, 0, 0);
    for (;;) {}
}
