

#include "syscall.h"


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
#define SYS_YIELD      34
#define SYS_PS         35
#define SYS_KILL       36
#define SYS_EXEC       37


static inline int syscall(int num, int arg1, int arg2, int arg3) {
    int ret;
    asm volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"(num), "b"(arg1), "c"(arg2), "d"(arg3)
        : "memory"
    );
    return ret;
}



int write(int fd, const char* buf, uint32_t len) {
    return syscall(SYS_WRITE, fd, (int)buf, len);
}

int read(int fd, char* buf, uint32_t len) {
    return syscall(SYS_READ, (int)buf, len, 0);
}

void exit(void) {
    syscall(SYS_EXIT, 0, 0, 0);
    while(1);  
}

void clear(void) {
    syscall(SYS_CLEAR, 0, 0, 0);
}



int disk_read(uint32_t lba, void* buffer) {
    return syscall(SYS_DISK_READ, lba, (int)buffer, 0);
}

int disk_write(uint32_t lba, const void* buffer) {
    return syscall(SYS_DISK_WRITE, lba, (int)buffer, 0);
}



void sleep_ms(uint32_t ms) {
    syscall(SYS_SLEEP, ms, 0, 0);
}



int open(const char* path, int flags) {
    return syscall(SYS_OPEN, (int)path, flags, 0);
}

int close(int fd) {
    return syscall(SYS_CLOSE, fd, 0, 0);
}

int file_read(int fd, void* buffer, uint32_t size) {
    return syscall(SYS_FILE_READ, fd, (int)buffer, size);
}

int file_write(int fd, const void* buffer, uint32_t size) {
    return syscall(SYS_FILE_WRITE, fd, (int)buffer, size);
}

int seek(int fd, int offset, int whence) {
    return syscall(SYS_SEEK, fd, offset, whence);
}

int unlink(const char* path) {
    return syscall(SYS_UNLINK, (int)path, 0, 0);
}

int mkdir(const char* path) {
    return syscall(SYS_MKDIR, (int)path, 0, 0);
}



int readdir(uint32_t cluster, uint32_t* index, file_info_t* info) {
    return syscall(SYS_READDIR, cluster, (int)index, (int)info);
}

int chdir(const char* path) {
    return syscall(SYS_CHDIR, (int)path, 0, 0);
}

int getcwd(char* buf, int size) {
    return syscall(SYS_GETCWD, (int)buf, size, 0);
}

uint32_t get_cwd_cluster(void) {
    return syscall(SYS_GET_CWD_CLUSTER, 0, 0, 0);
}



void yield(void) {
    syscall(SYS_YIELD, 0, 0, 0);
}

void ps(void) {
    syscall(SYS_PS, 0, 0, 0);
}

int kill(int pid) {
    return syscall(SYS_KILL, pid, 0, 0);
}

int exec(const char* path) {
    return syscall(SYS_EXEC, (int)path, 0, 0);
}



void reboot(void) {
    syscall(SYS_REBOOT, 0, 0, 0);
}

void shutdown(void) {
    syscall(SYS_SHUTDOWN, 0, 0, 0);
}

void halt(void) {
    syscall(SYS_HALT, 0, 0, 0);
}



int net_init(uint32_t ip, uint32_t gateway, uint32_t netmask) {
    return syscall(SYS_NET_INIT, ip, gateway, netmask);
}

int ping(uint32_t ip) {
    return syscall(SYS_PING, ip, 0, 0);
}

int ping_status(void) {
    return syscall(SYS_PING_STATUS, 0, 0, 0);
}

void ping_reset(void) {
    syscall(SYS_PING_RESET, 0, 0, 0);
}

int arp_request(uint32_t ip) {
    return syscall(SYS_ARP_REQUEST, ip, 0, 0);
}

int arp_lookup(uint32_t ip, uint8_t* mac) {
    return syscall(SYS_ARP_LOOKUP, ip, (int)mac, 0);
}

void arp_print_table(void) {
    syscall(SYS_ARP_PRINT, 0, 0, 0);
}

int arp_add(uint32_t ip, uint8_t* mac) {
    return syscall(SYS_ARP_ADD, ip, (int)mac, 0);
}

int arp_get_entry(int index, arp_entry_t* entry) {
    return syscall(SYS_ARP_GET_ENTRY, index, (int)entry, 0);
}

int net_status(net_status_t* status) {
    return syscall(SYS_NET_STATUS, (int)status, 0, 0);
}



int rtc_time(rtc_time_t* out) {
    return syscall(SYS_RTC_TIME, (int)out, 0, 0);
}

int timezone(int* out) {
    return syscall(SYS_TIMEZONE, (int)out, 0, 0);
}
