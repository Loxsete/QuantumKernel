

#ifndef USERSPACE_SYSCALL_H
#define USERSPACE_SYSCALL_H

#include <stdint.h>


#define O_RDONLY 0x01
#define O_WRONLY 0x02
#define O_RDWR   0x03
#define O_CREAT  0x04
#define O_TRUNC  0x08
#define O_APPEND 0x10


#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2


#define STDIN  0
#define STDOUT 1
#define STDERR 2


int write(int fd, const char* buf, uint32_t len);
int read(int fd, char* buf, uint32_t len);
void exit(void);
void clear(void);


int disk_read(uint32_t lba, void* buffer);
int disk_write(uint32_t lba, const void* buffer);


void sleep_ms(uint32_t ms);


int open(const char* path, int flags);
int close(int fd);
int file_read(int fd, void* buffer, uint32_t size);
int file_write(int fd, const void* buffer, uint32_t size);
int seek(int fd, int offset, int whence);
int unlink(const char* path);
int mkdir(const char* path);


typedef struct {
    char name[32];
    uint32_t size;
    uint8_t attr;
    uint32_t first_cluster;
} file_info_t;

int readdir(uint32_t cluster, uint32_t* index, file_info_t* info);
int chdir(const char* path);
int getcwd(char* buf, int size);
uint32_t get_cwd_cluster(void);


void yield(void);
void ps(void);
int kill(int pid);
int exec(const char* path);


void reboot(void);
void shutdown(void);
void halt(void);


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
} arp_entry_t;


int net_init(uint32_t ip, uint32_t gateway, uint32_t netmask);
int ping(uint32_t ip);
int ping_status(void);
void ping_reset(void);
int arp_request(uint32_t ip);
int arp_lookup(uint32_t ip, uint8_t* mac);
void arp_print_table(void);
int arp_add(uint32_t ip, uint8_t* mac);
int arp_get_entry(int index, arp_entry_t* entry);
int net_status(net_status_t* status);


typedef struct {
    uint8_t sec;
    uint8_t min;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint16_t year;
} rtc_time_t;

int rtc_time(rtc_time_t* out);
int timezone(int* out);

#endif 
