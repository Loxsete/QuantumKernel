#include <stdint.h>
#include "drivers/terminal.h"
#include "drivers/keyboard.h"
#include "drivers/timer.h"
#include "syscall/syscall.h"
#include "syscall/syscall_raw.h"
#include "drivers/ata.h"

typedef struct regs {
    uint32_t edi, esi, ebp, esp;
    uint32_t ebx, edx, ecx, eax;
    uint32_t gs, fs, es, ds;
} regs_t;

void syscall_dispatch(regs_t* r) {
    switch (r->eax) {
        case SYS_WRITE:
            if (r->ebx == 1) {
                const char* s = (const char*)r->ecx;
                for (uint32_t i = 0; i < r->edx; i++)
                    term_putc(s[i]);
            }
            r->eax = r->edx;
            break;
            
        case SYS_READ: {
            char* buf = (char*)r->ebx;
            uint32_t len = r->ecx;
            uint32_t i = 0;
            while (i < len) {
                int c = kbd_pop();
                if (c < 0)
                    break;
                buf[i++] = (char)c;
            }
            r->eax = i;
            break;
        }
        
        case SYS_EXIT:
            term_puts("\n[process exited]\n");
            for (;;)
                asm volatile("hlt");
            break;
            
        case SYS_CLEAR:
            term_clear();
            r->eax = 0;
            break;
            
        case SYS_DISK_READ: {
            uint32_t lba = r->ebx;
            uint8_t* buffer = (uint8_t*)r->ecx;
            
            if (buffer == 0) {
                r->eax = -1;
                break;
            }
            
            int result = ata_read_sector(lba, buffer);
            r->eax = result;
            break;
        }
        
        case SYS_DISK_WRITE: {
            uint32_t lba = r->ebx;
            const uint8_t* buffer = (const uint8_t*)r->ecx;
            
            if (buffer == 0) {
                r->eax = -1;
                break;
            }
            
            int result = ata_write_sector(lba, buffer);
            r->eax = result;
            break;
        }
        
        case SYS_SLEEP: {
            uint32_t ms = r->ebx;
            sleep(ms);
            r->eax = 0;
            break;
        }
        
        default:
            term_puts("[unknown syscall]\n");
            r->eax = -1;
            break;
    }
}

__attribute__((used))
int write(int fd, const char* buf, uint32_t len) {
    return syscall_invoke(SYS_WRITE, fd, (int)buf, len);
}

__attribute__((used))
int read(int fd, char* buf, uint32_t len) {
    if (fd != 0) return -1;
    return syscall_invoke(SYS_READ, (int)buf, len, 0);
}

__attribute__((used))
void exit(void) {
    syscall_invoke(SYS_EXIT, 0, 0, 0);
    for (;;) {}
}

__attribute__((used))
void clear(void) {
    syscall_invoke(SYS_CLEAR, 0, 0, 0);
}

__attribute__((used))
int disk_read(uint32_t lba, void* buffer) {
    return syscall_invoke(SYS_DISK_READ, lba, (int)buffer, 0);
}

__attribute__((used))
int disk_write(uint32_t lba, const void* buffer) {
    return syscall_invoke(SYS_DISK_WRITE, lba, (int)buffer, 0);
}

__attribute__((used))
void sleep_sys(uint32_t ms) {
    syscall_invoke(SYS_SLEEP, ms, 0, 0);
}
