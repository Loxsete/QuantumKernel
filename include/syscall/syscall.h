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

// FAT32 syscalls
#define SYS_OPEN       8
#define SYS_CLOSE      9
#define SYS_FILE_READ  10
#define SYS_FILE_WRITE 11
#define SYS_SEEK       12
#define SYS_UNLINK     13
#define SYS_MKDIR      14
#define SYS_READDIR    15 
// TIME
 
#define SYS_RTC_TIME     16  
#define SYS_TIMEZONE     17

// Basic I/O
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

#endif
