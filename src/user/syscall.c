#include <stdint.h>
#include "drivers/terminal.h"
#include "drivers/keyboard.h"
#include "drivers/timer.h"
#include "syscall/syscall.h"
#include "syscall/syscall_raw.h"
#include "drivers/ata.h"
#include "fs/fat32.h"

typedef struct regs {
    uint32_t edi, esi, ebp, esp;
    uint32_t ebx, edx, ecx, eax;
    uint32_t gs, fs, es, ds;
} regs_t;

#define MAX_FILES 16
static fat32_file_t file_table[MAX_FILES];
static int file_used[MAX_FILES] = {0};

static int alloc_fd(void) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (!file_used[i]) {
            file_used[i] = 1;
            return i;
        }
    }
    return -1;
}

static void free_fd(int fd) {
    if (fd >= 0 && fd < MAX_FILES) {
        file_used[fd] = 0;
    }
}

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
        
        // FAT32 syscalls
        case SYS_OPEN: {
            const char* path = (const char*)r->ebx;
            int flags = r->ecx;
            
            int fd = alloc_fd();
            if (fd < 0) {
                r->eax = -1;
                break;
            }
            
            int result = fat32_open(&file_table[fd], path, (uint8_t)flags);
            if (result != 0) {
                free_fd(fd);
                r->eax = -1;
            } else {
                r->eax = fd;
            }
            break;
        }
        
        case SYS_CLOSE: {
            int fd = r->ebx;
            
            if (fd < 0 || fd >= MAX_FILES || !file_used[fd]) {
                r->eax = -1;
                break;
            }
            
            int result = fat32_close(&file_table[fd]);
            free_fd(fd);
            r->eax = result;
            break;
        }
        
        case SYS_FILE_READ: {
            int fd = r->ebx;
            void* buffer = (void*)r->ecx;
            uint32_t size = r->edx;
            
            if (fd < 0 || fd >= MAX_FILES || !file_used[fd]) {
                r->eax = -1;
                break;
            }
            
            int result = fat32_read(&file_table[fd], buffer, size);
            r->eax = result;
            break;
        }
        
        case SYS_FILE_WRITE: {
            int fd = r->ebx;
            const void* buffer = (const void*)r->ecx;
            uint32_t size = r->edx;
            
            if (fd < 0 || fd >= MAX_FILES || !file_used[fd]) {
                r->eax = -1;
                break;
            }
            
            int result = fat32_write(&file_table[fd], buffer, size);
            r->eax = result;
            break;
        }
        
        case SYS_SEEK: {
            int fd = r->ebx;
            int offset = r->ecx;
            int whence = r->edx;
            
            if (fd < 0 || fd >= MAX_FILES || !file_used[fd]) {
                r->eax = -1;
                break;
            }
            
            int result = fat32_seek(&file_table[fd], offset, (uint8_t)whence);
            r->eax = result;
            break;
        }
        
        case SYS_UNLINK: {
            const char* path = (const char*)r->ebx;
            int result = fat32_unlink(path);
            r->eax = result;
            break;
        }
        
        case SYS_MKDIR: {
            const char* path = (const char*)r->ebx;
            int result = fat32_mkdir(path);
            r->eax = result;
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

// FAT32 wrappers
__attribute__((used))
int open(const char* path, int flags) {
    return syscall_invoke(SYS_OPEN, (int)path, flags, 0);
}

__attribute__((used))
int close(int fd) {
    return syscall_invoke(SYS_CLOSE, fd, 0, 0);
}

__attribute__((used))
int file_read(int fd, void* buffer, uint32_t size) {
    return syscall_invoke(SYS_FILE_READ, fd, (int)buffer, size);
}

__attribute__((used))
int file_write(int fd, const void* buffer, uint32_t size) {
    return syscall_invoke(SYS_FILE_WRITE, fd, (int)buffer, size);
}

__attribute__((used))
int seek(int fd, int offset, int whence) {
    return syscall_invoke(SYS_SEEK, fd, offset, whence);
}

__attribute__((used))
int unlink(const char* path) {
    return syscall_invoke(SYS_UNLINK, (int)path, 0, 0);
}

__attribute__((used))
int mkdir(const char* path) {
    return syscall_invoke(SYS_MKDIR, (int)path, 0, 0);
}


