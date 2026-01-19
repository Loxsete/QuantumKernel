#ifndef LIBC_SYSCALL_H
#define LIBC_SYSCALL_H

#include <stdint.h>


#define O_RDONLY 0x01
#define O_WRONLY 0x02
#define O_RDWR   0x03
#define O_CREAT  0x04
#define O_TRUNC  0x08


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
int readdir(uint32_t cluster, uint32_t* index, void* info);
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


int net_init(uint32_t ip, uint32_t gateway, uint32_t netmask);
int ping(uint32_t ip);
int ping_status(void);
void ping_reset(void);
int arp_request(uint32_t ip);
int arp_lookup(uint32_t ip, uint8_t* mac);
void arp_print_table(void);
int arp_add(uint32_t ip, uint8_t* mac);

#endif 
