#ifndef FAT32_H
#define FAT32_H

#include <stdint.h>
#include "lib/stddef.h"
#include "fs/vfs.h"

#define FAT32_SECTOR_SIZE 512
#define FAT32_MAX_PATH 260
#define FAT32_MAX_FILENAME 255

// FAT32 Boot Sector
typedef struct {
    uint8_t  jmp[3];
    uint8_t  oem[8];
    uint16_t bytes_per_sector;
    uint8_t  sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t  num_fats;
    uint16_t root_entries;
    uint16_t total_sectors_16;
    uint8_t  media_type;
    uint16_t fat_size_16;
    uint16_t sectors_per_track;
    uint16_t num_heads;
    uint32_t hidden_sectors;
    uint32_t total_sectors_32;
    
    // FAT32 Extended
    uint32_t fat_size_32;
    uint16_t ext_flags;
    uint16_t fs_version;
    uint32_t root_cluster;
    uint16_t fs_info;
    uint16_t backup_boot;
    uint8_t  reserved[12];
    uint8_t  drive_number;
    uint8_t  reserved1;
    uint8_t  boot_sig;
    uint32_t volume_id;
    uint8_t  volume_label[11];
    uint8_t  fs_type[8];
} __attribute__((packed)) fat32_boot_sector_t;

// FAT32 FSInfo Sector
typedef struct {
    uint32_t lead_sig;
    uint8_t  reserved1[480];
    uint32_t struct_sig;
    uint32_t free_count;
    uint32_t next_free;
    uint8_t  reserved2[12];
    uint32_t trail_sig;
} __attribute__((packed)) fat32_fsinfo_t;

// Directory Entry
typedef struct {
    uint8_t  name[11];
    uint8_t  attr;
    uint8_t  nt_reserved;
    uint8_t  create_time_tenth;
    uint16_t create_time;
    uint16_t create_date;
    uint16_t access_date;
    uint16_t first_cluster_high;
    uint16_t write_time;
    uint16_t write_date;
    uint16_t first_cluster_low;
    uint32_t file_size;
} __attribute__((packed)) fat32_dirent_t;

// Long File Name Entry
typedef struct {
    uint8_t  order;
    uint16_t name1[5];
    uint8_t  attr;
    uint8_t  type;
    uint8_t  checksum;
    uint16_t name2[6];
    uint16_t first_cluster_low;
    uint16_t name3[2];
} __attribute__((packed)) fat32_lfn_entry_t;

// File Attributes
#define FAT32_ATTR_READ_ONLY 0x01
#define FAT32_ATTR_HIDDEN    0x02
#define FAT32_ATTR_SYSTEM    0x04
#define FAT32_ATTR_VOLUME_ID 0x08
#define FAT32_ATTR_DIRECTORY 0x10
#define FAT32_ATTR_ARCHIVE   0x20
#define FAT32_ATTR_LFN       0x0F

// Special Cluster Values
#define FAT32_FREE_CLUSTER   0x00000000
#define FAT32_BAD_CLUSTER    0x0FFFFFF7
#define FAT32_EOC_MIN        0x0FFFFFF8
#define FAT32_EOC_MAX        0x0FFFFFFF

// File Modes
#define FAT32_O_RDONLY       0x01
#define FAT32_O_WRONLY       0x02
#define FAT32_O_RDWR         0x03
#define FAT32_O_CREAT        0x04
#define FAT32_O_TRUNC        0x08
#define FAT32_O_APPEND       0x10

// Seek Modes
#define FAT32_SEEK_SET       0
#define FAT32_SEEK_CUR       1
#define FAT32_SEEK_END       2

// File Handle
typedef struct {
    uint32_t first_cluster;
    uint32_t current_cluster;
    uint32_t file_size;
    uint32_t position;
    uint32_t dir_cluster;
    uint32_t dir_entry_offset;
    uint8_t  flags;
    uint8_t  is_open;
    uint8_t  attr;
} fat32_file_t;

// Directory Entry Info
typedef struct {
    char     name[FAT32_MAX_FILENAME + 1];
    uint32_t size;
    uint8_t  attr;
    uint32_t first_cluster;
    uint16_t create_date;
    uint16_t create_time;
    uint16_t write_date;
    uint16_t write_time;
} fat32_file_info_t;

// FAT32 Filesystem Structure
typedef struct {
    fat32_boot_sector_t boot;
    fat32_fsinfo_t fsinfo;
    uint32_t fat_begin_lba;
    uint32_t cluster_begin_lba;
    uint32_t root_dir_first_cluster;
    uint32_t sectors_per_cluster;
    uint32_t bytes_per_cluster;
    uint32_t total_clusters;
    uint8_t  num_fats;
} fat32_fs_t;

// Main API Functions
int fat32_init(void);
int fat32_format(uint32_t total_sectors);
int fat32_mount(void);

// File Operations
int fat32_open(fat32_file_t* file, const char* path, uint8_t flags);
int fat32_close(fat32_file_t* file);
int fat32_read(fat32_file_t* file, void* buffer, uint32_t size);
int fat32_write(fat32_file_t* file, const void* buffer, uint32_t size);
int fat32_seek(fat32_file_t* file, int32_t offset, uint8_t whence);
int fat32_tell(fat32_file_t* file);

// Directory Operations
int fat32_mkdir(const char* path);
int fat32_rmdir(const char* path);
int fat32_opendir(const char* path, uint32_t* cluster);
int fat32_readdir(uint32_t cluster, uint32_t* index, fat32_file_info_t* info);

// File Management
int fat32_unlink(const char* path);
int fat32_rename(const char* old_path, const char* new_path);
int fat32_stat(const char* path, fat32_file_info_t* info);

// Utility Functions
void fat32_list_dir(const char* path);
void fat32_print_info(void);
uint32_t fat32_get_root_cluster(void); 
vfs_ops_t* fat32_get_ops(void);

extern uint32_t g_current_dir_cluster;
int fat32_chdir(const char* path);           
char* fat32_getcwd(char* buf, size_t size);  
int fat32_opendir_path(const char* path, uint32_t* cluster);  

int fat32_get_current_path(char* buffer, int buffer_size);

#endif
