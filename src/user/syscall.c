

#include <stdint.h>
#include "drivers/terminal.h"
#include "drivers/keyboard.h"
#include "drivers/timer.h"
#include "drivers/power.h"
#include "syscall/syscall.h"
#include "syscall/syscall_raw.h"
#include "lib/libc.h"
#include "drivers/ata.h"
#include "fs/fat32.h"
#include "drivers/rtc.h"
#include "drivers/net/ip.h"
#include "kernel/task.h"
#include "drivers/net/icmp.h"
#include "kernel/exec.h"
#include "drivers/net/arp.h"
#include "drivers/net/ethernet.h"

typedef struct regs {
    uint32_t edi, esi, ebp, esp;
    uint32_t ebx, edx, ecx, eax;
    uint32_t gs, fs, es, ds;
} regs_t;


#define MAX_FILES 16
static fat32_file_t file_table[MAX_FILES];
static int file_used[MAX_FILES] = {0};


static uint32_t net_ip = 0;
static uint32_t net_gateway = 0;
static uint32_t net_netmask = 0;
static int net_initialized = 0;

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

        case SYS_EXEC: {
            const char* path = (const char*)r->ebx;
            r->eax = exec(path);
            break;
        }
        
        case SYS_GETCWD: {
            char* buffer = (char*)r->ebx;
            int size = r->ecx;
            if (!buffer || size <= 0) {
                r->eax = -1;
                break;
            }
            
            int result = fat32_get_current_path(buffer, size);
            r->eax = result;
            break;
        }

        case SYS_YIELD:
            task_schedule();
            r->eax = 0;
            break;

        case SYS_PS: {
            task_t* t = task_get_list();
            task_t* start = t;
        
            term_puts("PID   STATE     NAME\n");
        
            do {
                char buf[32];
        
                itoa(t->pid, buf, 10);
                term_puts(buf);
                term_puts("   ");
        
                if (t->state == TASK_RUNNING) term_puts("RUN      ");
                else if (t->state == TASK_SLEEPING) term_puts("SLEEP    ");
                else term_puts("ZOMB     ");
        
                term_puts(t->name);
                term_puts("\n");
        
                t = t->next;
            } while (t != start);
        
            r->eax = 0;
            break;
        }
        
        case SYS_KILL:
            r->eax = task_kill(r->ebx); 
            break;
        
        case SYS_EXIT:
            task_exit();
            __builtin_unreachable();
        
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
        
        case SYS_SLEEP:
            task_sleep(r->ebx);
            r->eax = 0;
            break;
        
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

        case SYS_READDIR: {
            uint32_t cluster = r->ebx;
            uint32_t* index = (uint32_t*)r->ecx;
            fat32_file_info_t* info = (fat32_file_info_t*)r->edx;
            if (!index || !info) {
                r->eax = -1;
                break;
            }
            int result = fat32_readdir(cluster, index, info);
            r->eax = result;
            break;
        }

        case SYS_CHDIR: {
            const char* path = (const char*)r->ebx;
            int result = fat32_chdir(path);
            r->eax = result;
            break;
        }
        
        case SYS_GET_CWD_CLUSTER: {
            extern uint32_t g_current_dir_cluster;
            r->eax = g_current_dir_cluster;
            break;
        }
        
        case SYS_RTC_TIME: {
            rtc_time_t* out = (rtc_time_t*)r->ebx;
            if (out)
                rtc_time_sys(out);
            r->eax = 0;
            break;
        }
        
        case SYS_TIMEZONE: {
            int* out = (int*)r->ebx;
            if (out)
                *out = timezone_sys();
            r->eax = 0;
            break;
        }

        case SYS_NET_INIT: {
            net_ip = r->ebx;
            net_gateway = r->ecx;
            net_netmask = r->edx;
            ip_init(net_ip);
            arp_init(net_ip);
            net_initialized = 1;
            r->eax = 0;
            break;
        }

        case SYS_PING: {
            uint32_t ip = r->ebx;
            icmp_ping(ip);
            r->eax = 0;
            break;
        }

        case SYS_PING_STATUS: {
            r->eax = icmp_get_status();
            break;
        }

        case SYS_PING_RESET: {
            icmp_reset_status();
            r->eax = 0;
            break;
        }

        case SYS_ARP_REQUEST: {
            uint32_t ip = r->ebx;
            arp_request(ip);
            r->eax = 0;
            break;
        }

        case SYS_ARP_LOOKUP: {
            uint32_t ip = r->ebx;
            uint8_t* mac = (uint8_t*)r->ecx;
            if (!mac) {
                r->eax = -1;
                break;
            }
            r->eax = arp_lookup(ip, mac);
            break;
        }

        case SYS_ARP_PRINT: {
            extern void arp_print_table(void);
            arp_print_table();
            r->eax = 0;
            break;
        }

        case SYS_ARP_ADD: {
            uint32_t ip = r->ebx;
            uint8_t* mac = (uint8_t*)r->ecx;
            if (!mac) {
                r->eax = -1;
                break;
            }
            extern void arp_add_entry(uint32_t ip, uint8_t *mac);
            arp_add_entry(ip, mac);
            r->eax = 0;
            break;
        }

        case SYS_ARP_GET_ENTRY: {
            int index = r->ebx;
            arp_entry_sys_t* entry = (arp_entry_sys_t*)r->ecx;
            if (!entry || index < 0 || index >= 16) {
                r->eax = -1;
                break;
            }
            extern int arp_get_entry_by_index(int index, uint32_t* ip, uint8_t* mac, int* valid);
            int result = arp_get_entry_by_index(index, &entry->ip, entry->mac, &entry->valid);
            r->eax = result;
            break;
        }

        case SYS_NET_STATUS: {
            net_status_t* status = (net_status_t*)r->ebx;
            if (!status) {
                r->eax = -1;
                break;
            }
            if (!net_initialized) {
                r->eax = -1;
                break;
            }
            status->ip = net_ip;
            status->gateway = net_gateway;
            status->netmask = net_netmask;
            uint8_t* mac = eth_mac();
            for (int i = 0; i < 6; i++) {
                status->mac[i] = mac[i];
            }
            status->link_up = 1;
            status->tx_packets = 0;
            status->rx_packets = 0;
            status->tx_errors = 0;
            status->rx_errors = 0;
            r->eax = 0;
            break;
        }

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

        case SYS_REBOOT:
            term_puts("\nRebooting system...\n");
            power_reboot();
            break;
            
        case SYS_SHUTDOWN:
            term_puts("\nShutting down...\n");
            power_shutdown();
            break;
            
        case SYS_HALT:
            term_puts("\nSystem halted.\n");
            power_halt();
            break;

        default:
            term_puts("[unknown syscall]\n");
            r->eax = -1;
            break;
    }
}
