#ifndef SYSCALL_H
#define SYSCALL_H
#include <stdint.h>
#define SYS_WRITE      1
#define SYS_READ       2
#define SYS_EXIT       3
#define SYS_CLEAR      4
#define SYS_DISK_READ  5
#define SYS_DISK_WRITE 6
#define SYS_SLEEP      7
#define SYS_OPEN       8
#define SYS_CLOSE      9
#define SYS_FILE_READ  10
#define SYS_FILE_WRITE 11
#define SYS_SEEK       12
#define SYS_UNLINK     13
#define SYS_MKDIR      14
#define SYS_READDIR    15
#define SYS_RTC_TIME   16
#define SYS_TIMEZONE   17
#define SYS_CHDIR      18
#define SYS_GETCWD     19
#define SYS_GET_CWD_CLUSTER 20
#define SYS_NET_INIT   21
#define SYS_PING       22
#define SYS_PING_STATUS 23
#define SYS_PING_RESET 24
#define SYS_ARP_REQUEST 25
#define SYS_ARP_LOOKUP  26
#define SYS_ARP_PRINT   27
#define SYS_NET_STATUS  28
#define SYS_ARP_ADD     29
#define SYS_ARP_GET_ENTRY 30
#define SYS_REBOOT     31
#define SYS_SHUTDOWN   32
#define SYS_HALT       33
#define SYS_YIELD 34
#define SYS_PS 35
#define SYS_KILL 36
#define SYS_EXEC 37



typedef struct {
    uint32_t ip;
    uint32_t gateway;
    uint32_t netmask;
    uint8_t mac[6];
    int link_up;
    uint32_t tx_packets;
    uint32_t rx_packets;
    uint32_t tx_errors;
    uint32_t rx_errors;
} net_status_t;
typedef struct {
    uint32_t ip;
    uint8_t mac[6];
    int valid;
} arp_entry_sys_t;
int write(int fd, const char* buf, uint32_t len);
int read(int fd, char* buf, uint32_t len);
void exit(void);
void clear(void);
int disk_read(uint32_t lba, void* buffer);
int disk_write(uint32_t lba, const void* buffer);
void sleep_sys(uint32_t ms);
int open(const char* path, int flags);
int close(int fd);
int file_read(int fd, void* buffer, uint32_t size);
int file_write(int fd, const void* buffer, uint32_t size);
int seek(int fd, int offset, int whence);
int unlink(const char* path);
int mkdir(const char* path);
int readdir_sys(uint32_t cluster, uint32_t* index, void* info);
int chdir_sys(const char* path);
uint32_t get_cwd_cluster_sys(void);
int net_init_sys(uint32_t ip, uint32_t gateway, uint32_t netmask);
int ping_sys(uint32_t ip);
int ping_status_sys(void);
void ping_reset_sys(void);
int arp_request_sys(uint32_t ip);
int arp_lookup_sys(uint32_t ip, uint8_t* mac);
void arp_print_table_sys(void);
int get_net_status_sys(net_status_t* status);
int arp_add_sys(uint32_t ip, uint8_t* mac);
int arp_get_entry_sys(int index, arp_entry_sys_t* entry);
void reboot_sys(void);
void shutdown_sys(void);
void ps_sys(void);
int kill(int pid);
void halt_sys(void);
int getcwd(char* buf, int size);
int exec_sys(const char* path);
#endif
