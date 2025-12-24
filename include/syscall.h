#pragma once
#include <stdint.h>

#define SYS_WRITE      1
#define SYS_EXIT       2
#define SYS_READ       3
#define SYS_CLEAR      4
#define SYS_DISK_READ  5  
#define SYS_DISK_WRITE 6  

int write(int fd, const char* buf, uint32_t len);
int read(int fd, char* buf, uint32_t len);
void exit(void);
void clear(void);
int disk_read(uint32_t lba, void* buffer);   
int disk_write(uint32_t lba, const void* buffer); 
