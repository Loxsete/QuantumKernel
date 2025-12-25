#include "fs/fat32.h"
#include "drivers/ata.h"
#include "drivers/terminal.h"
#include "lib/libc.h"
#include "lib/string.h"
#include <stddef.h>

static fat32_fs_t g_fs;
static uint8_t sector_buffer[FAT32_SECTOR_SIZE];
static uint8_t cluster_buffer[128 * FAT32_SECTOR_SIZE]; 

static uint32_t fat32_read_fat(uint32_t cluster) {
    if (cluster < 2 || cluster >= g_fs.total_clusters + 2)
        return FAT32_BAD_CLUSTER;

    uint32_t fat_offset = cluster * 4;
    uint32_t fat_sector = g_fs.fat_begin_lba + (fat_offset / FAT32_SECTOR_SIZE);
    uint32_t entry_offset = fat_offset % FAT32_SECTOR_SIZE;

    if (ata_read_sector(fat_sector, sector_buffer) != 0)
        return FAT32_BAD_CLUSTER;

    uint32_t value = *(uint32_t*)(sector_buffer + entry_offset);
    return value & 0x0FFFFFFF;
}


static int fat32_write_fat(uint32_t cluster, uint32_t value) {
    if (cluster < 2 || cluster >= g_fs.total_clusters + 2)
        return -1;

    value &= 0x0FFFFFFF;

    for (uint8_t i = 0; i < g_fs.num_fats; i++) {
        uint32_t fat_offset = cluster * 4;
        uint32_t fat_sector = g_fs.fat_begin_lba +
                              (i * g_fs.boot.fat_size_32) +
                              (fat_offset / FAT32_SECTOR_SIZE);
        uint32_t entry_offset = fat_offset % FAT32_SECTOR_SIZE;

        if (ata_read_sector(fat_sector, sector_buffer) != 0)
            return -1;

        *(uint32_t*)(sector_buffer + entry_offset) = value;

        if (ata_write_sector(fat_sector, sector_buffer) != 0)
            return -1;
    }

    return 0;
}


static uint32_t fat32_alloc_cluster(void) {
    uint32_t cluster = g_fs.fsinfo.next_free;
    if (cluster < 2)
        cluster = 2;

    for (uint32_t i = 0; i < g_fs.total_clusters; i++) {
        if (cluster >= g_fs.total_clusters + 2)
            cluster = 2;

        uint32_t value = fat32_read_fat(cluster);
        if (value == FAT32_FREE_CLUSTER) {
            fat32_write_fat(cluster, FAT32_EOC_MAX);
            g_fs.fsinfo.next_free = cluster + 1;
            return cluster;
        }
        cluster++;
    }

    return 0;
}

static void fat32_free_cluster_chain(uint32_t cluster) {
    while (cluster >= 2 && cluster < FAT32_EOC_MIN &&
           cluster < g_fs.total_clusters + 2) {
        uint32_t next = fat32_read_fat(cluster);
        fat32_write_fat(cluster, FAT32_FREE_CLUSTER);
        cluster = next;
    }
}

static uint32_t fat32_cluster_to_lba(uint32_t cluster) {
    if (cluster < 2 || cluster >= g_fs.total_clusters + 2)
        return 0;

    return g_fs.cluster_begin_lba +
           ((cluster - 2) * g_fs.sectors_per_cluster);
}



static int fat32_read_cluster(uint32_t cluster, void* buffer) {
    uint32_t lba = fat32_cluster_to_lba(cluster);
    if (lba == 0)
        return -1;
    
    for (uint32_t i = 0; i < g_fs.sectors_per_cluster; i++) {
        if (ata_read_sector(lba + i, (uint8_t*)buffer + (i * FAT32_SECTOR_SIZE)) != 0)
            return -1;
    }
    return 0;
}


static int fat32_write_cluster(uint32_t cluster, const void* buffer) {
    uint32_t lba = fat32_cluster_to_lba(cluster);
    if (lba == 0)
        return -1;
    
    for (uint32_t i = 0; i < g_fs.sectors_per_cluster; i++) {
        if (ata_write_sector(lba + i, (uint8_t*)buffer + (i * FAT32_SECTOR_SIZE)) != 0)
            return -1;
    }
    return 0;
}


static uint8_t fat32_lfn_checksum(const uint8_t* short_name) __attribute__((unused));
static uint8_t fat32_lfn_checksum(const uint8_t* short_name) {
    uint8_t sum = 0;
    for (int i = 0; i < 11; i++) {
        sum = ((sum & 1) ? 0x80 : 0) + (sum >> 1) + short_name[i];
    }
    return sum;
}


static void fat32_to_short_name(const char* name, uint8_t* short_name) {
    memset(short_name, ' ', 11);
    
    const char* dot = NULL;
    for (const char* p = name; *p; p++) {
        if (*p == '.')
            dot = p;
    }
    
    int name_len = dot ? (dot - name) : strlen(name);
    int ext_len = dot ? strlen(dot + 1) : 0;
    
    if (name_len > 8) name_len = 8;
    if (ext_len > 3) ext_len = 3;
    
    for (int i = 0; i < name_len; i++) {
        char c = name[i];
        if (c >= 'a' && c <= 'z')
            c -= 32;
        short_name[i] = c;
    }
    
    if (dot) {
        for (int i = 0; i < ext_len; i++) {
            char c = dot[1 + i];
            if (c >= 'a' && c <= 'z')
                c -= 32;
            short_name[8 + i] = c;
        }
    }
}


static int fat32_find_entry(uint32_t dir_cluster, const char* name,
                           fat32_dirent_t* entry,
                           uint32_t* entry_cluster,
                           uint32_t* entry_offset) {
    uint8_t short_name[11];
    fat32_to_short_name(name, short_name);

    uint32_t cluster = dir_cluster;

    while (cluster >= 2 &&
           cluster < FAT32_EOC_MIN &&
           cluster < g_fs.total_clusters + 2) {

        if (fat32_read_cluster(cluster, cluster_buffer) != 0)
            return -1;

        fat32_dirent_t* entries = (fat32_dirent_t*)cluster_buffer;
        uint32_t count = g_fs.bytes_per_cluster / sizeof(fat32_dirent_t);

        for (uint32_t i = 0; i < count; i++) {
            if (entries[i].name[0] == 0x00)
                return -1;

            if (entries[i].name[0] == 0xE5)
                continue;

            if (entries[i].attr == FAT32_ATTR_LFN)
                continue;

            if (memcmp(entries[i].name, short_name, 11) == 0) {
                if (entry) memcpy(entry, &entries[i], sizeof(*entry));
                if (entry_cluster) *entry_cluster = cluster;
                if (entry_offset) *entry_offset = i * sizeof(fat32_dirent_t);
                return 0;
            }
        }

        cluster = fat32_read_fat(cluster);
    }

    return -1;
}



static int fat32_create_entry(uint32_t dir_cluster, const char* name, 
                             uint8_t attr, uint32_t first_cluster) {
    uint8_t short_name[11];
    fat32_to_short_name(name, short_name);
    
    uint32_t cluster = dir_cluster;
    
    while (cluster >= 2 && cluster < FAT32_EOC_MIN) {
        if (fat32_read_cluster(cluster, cluster_buffer) != 0)
            return -1;
        
        fat32_dirent_t* entries = (fat32_dirent_t*)cluster_buffer;
        uint32_t entries_per_cluster = g_fs.bytes_per_cluster / sizeof(fat32_dirent_t);
        
        for (uint32_t i = 0; i < entries_per_cluster; i++) {
            if (entries[i].name[0] == 0x00 || entries[i].name[0] == 0xE5) {
                memset(&entries[i], 0, sizeof(fat32_dirent_t));
                memcpy(entries[i].name, short_name, 11);
                entries[i].attr = attr;
                entries[i].first_cluster_low = first_cluster & 0xFFFF;
                entries[i].first_cluster_high = (first_cluster >> 16) & 0xFFFF;
                
                return fat32_write_cluster(cluster, cluster_buffer);
            }
        }
        
        uint32_t next = fat32_read_fat(cluster);
        if (next >= FAT32_EOC_MIN) {
            uint32_t new_cluster = fat32_alloc_cluster();
            if (new_cluster == 0)
                return -1;
            
            fat32_write_fat(cluster, new_cluster);
            memset(cluster_buffer, 0, g_fs.bytes_per_cluster);
            fat32_write_cluster(new_cluster, cluster_buffer);
            cluster = new_cluster;
        } else {
            cluster = next;
        }
    }
    
    return -1;
}


int fat32_init(void) {
    memset(&g_fs, 0, sizeof(fat32_fs_t));
    return 0;
}


int fat32_format(uint32_t total_sectors) {
    term_puts("Formatting FAT32 filesystem...\n");
    
    memset(&g_fs.boot, 0, sizeof(fat32_boot_sector_t));
    
    g_fs.boot.jmp[0] = 0xEB;
    g_fs.boot.jmp[1] = 0x58;
    g_fs.boot.jmp[2] = 0x90;
    memcpy(g_fs.boot.oem, "MYOS    ", 8);
    
    g_fs.boot.bytes_per_sector = FAT32_SECTOR_SIZE;
    g_fs.boot.sectors_per_cluster = 8;
    g_fs.boot.reserved_sectors = 32;
    g_fs.boot.num_fats = 2;
    g_fs.boot.root_entries = 0;
    g_fs.boot.total_sectors_16 = 0;
    g_fs.boot.media_type = 0xF8;
    g_fs.boot.fat_size_16 = 0;
    g_fs.boot.sectors_per_track = 63;
    g_fs.boot.num_heads = 255;
    g_fs.boot.hidden_sectors = 0;
    g_fs.boot.total_sectors_32 = total_sectors;
    
    uint32_t fat_size = ((total_sectors - 32) / (8 + (2 * 128))) + 1;
    g_fs.boot.fat_size_32 = fat_size;
    g_fs.boot.ext_flags = 0;
    g_fs.boot.fs_version = 0;
    g_fs.boot.root_cluster = 2;
    g_fs.boot.fs_info = 1;
    g_fs.boot.backup_boot = 6;
    g_fs.boot.drive_number = 0x80;
    g_fs.boot.boot_sig = 0x29;
    g_fs.boot.volume_id = 0x12345678;
    memcpy(g_fs.boot.volume_label, "MYOS DISK  ", 11);
    memcpy(g_fs.boot.fs_type, "FAT32   ", 8);
    
    memset(sector_buffer, 0, FAT32_SECTOR_SIZE);
    memcpy(sector_buffer, &g_fs.boot, sizeof(fat32_boot_sector_t));
    sector_buffer[510] = 0x55;
    sector_buffer[511] = 0xAA;
    
    if (ata_write_sector(0, sector_buffer) != 0) {
        term_puts("Failed to write boot sector\n");
        return -1;
    }
    
    memset(&g_fs.fsinfo, 0, sizeof(fat32_fsinfo_t));
    g_fs.fsinfo.lead_sig = 0x41615252;
    g_fs.fsinfo.struct_sig = 0x61417272;
    g_fs.fsinfo.free_count = 0xFFFFFFFF;
    g_fs.fsinfo.next_free = 3;
    g_fs.fsinfo.trail_sig = 0xAA550000;
    
    memset(sector_buffer, 0, FAT32_SECTOR_SIZE);
    memcpy(sector_buffer, &g_fs.fsinfo, sizeof(fat32_fsinfo_t));
    
    if (ata_write_sector(1, sector_buffer) != 0) {
        term_puts("Failed to write FSInfo\n");
        return -1;
    }
    
    uint32_t fat_begin = 32;
    memset(sector_buffer, 0, FAT32_SECTOR_SIZE);
    
    for (uint8_t f = 0; f < 2; f++) {
        uint32_t fat_lba = fat_begin + (f * fat_size);
        
        for (uint32_t i = 0; i < fat_size; i++) {
            if (i == 0) {
                sector_buffer[0] = 0xF8;
                sector_buffer[1] = 0xFF;
                sector_buffer[2] = 0xFF;
                sector_buffer[3] = 0x0F;
                sector_buffer[4] = 0xFF;
                sector_buffer[5] = 0xFF;
                sector_buffer[6] = 0xFF;
                sector_buffer[7] = 0x0F;
                sector_buffer[8] = 0xFF;
                sector_buffer[9] = 0xFF;
                sector_buffer[10] = 0xFF;
                sector_buffer[11] = 0x0F;
            } else {
                memset(sector_buffer, 0, FAT32_SECTOR_SIZE);
            }
            
            if (ata_write_sector(fat_lba + i, sector_buffer) != 0) {
                term_puts("Failed to write FAT\n");
                return -1;
            }
        }
    }
    
    uint32_t data_begin = fat_begin + (2 * fat_size);
    memset(sector_buffer, 0, FAT32_SECTOR_SIZE);
    
    for (uint32_t i = 0; i < 8; i++) {
        if (ata_write_sector(data_begin + i, sector_buffer) != 0) {
            term_puts("Failed to clear root directory\n");
            return -1;
        }
    }
    
    term_puts("FAT32 format complete\n");
    return fat32_mount();
}


int fat32_mount(void) {
    term_puts("Mounting FAT32 filesystem...\n");
    
    if (ata_read_sector(0, sector_buffer) != 0) {
        term_puts("Failed to read boot sector\n");
        return -1;
    }
    
    memcpy(&g_fs.boot, sector_buffer, sizeof(fat32_boot_sector_t));
    
    if (g_fs.boot.bytes_per_sector != FAT32_SECTOR_SIZE) {
        term_puts("Invalid sector size\n");
        return -1;
    }
    
    if (ata_read_sector(g_fs.boot.fs_info, sector_buffer) != 0) {
        term_puts("Failed to read FSInfo\n");
        return -1;
    }
    
    memcpy(&g_fs.fsinfo, sector_buffer, sizeof(fat32_fsinfo_t));
    
    g_fs.fat_begin_lba = g_fs.boot.reserved_sectors;
    g_fs.cluster_begin_lba = g_fs.fat_begin_lba + 
                             (g_fs.boot.num_fats * g_fs.boot.fat_size_32);
	g_fs.root_dir_first_cluster = g_fs.boot.root_cluster;
	
	if (g_fs.root_dir_first_cluster < 2) {
	    term_puts("Invalid root cluster\n");
	    return -1;
	}
	
    g_fs.sectors_per_cluster = g_fs.boot.sectors_per_cluster;
    g_fs.bytes_per_cluster = g_fs.sectors_per_cluster * FAT32_SECTOR_SIZE;
    g_fs.num_fats = g_fs.boot.num_fats;

    
    
    uint32_t data_sectors = g_fs.boot.total_sectors_32 - g_fs.cluster_begin_lba;
    g_fs.total_clusters = data_sectors / g_fs.sectors_per_cluster;
    
    term_puts("FAT32 mounted successfully\n");
    return 0;
}


int fat32_open(fat32_file_t* file, const char* path, uint8_t flags) {
    if (!file || !path)
        return -1;
    
    memset(file, 0, sizeof(fat32_file_t));
    
    fat32_dirent_t entry;
    uint32_t dir_cluster = g_fs.root_dir_first_cluster;
    
    if (fat32_find_entry(dir_cluster, path, &entry, 
                        &file->dir_cluster, &file->dir_entry_offset) == 0) {
        file->first_cluster = ((uint32_t)entry.first_cluster_high << 16) | 
                             entry.first_cluster_low;
        file->current_cluster = file->first_cluster;
        file->file_size = entry.file_size;
        file->attr = entry.attr;
        
        if (flags & FAT32_O_TRUNC) {
            if (file->first_cluster >= 2) {
                fat32_free_cluster_chain(file->first_cluster);
                file->first_cluster = 0;
                file->current_cluster = 0;
                file->file_size = 0;
            }
        }
    } else {
        if (!(flags & FAT32_O_CREAT))
            return -1;
        
        uint32_t new_cluster = fat32_alloc_cluster();
        if (new_cluster == 0)
            return -1;
        
        if (fat32_create_entry(dir_cluster, path, FAT32_ATTR_ARCHIVE, new_cluster) != 0) {
            fat32_free_cluster_chain(new_cluster);
            return -1;
        }
        
        file->first_cluster = new_cluster;
        file->current_cluster = new_cluster;
        file->file_size = 0;
        file->attr = FAT32_ATTR_ARCHIVE;
    }
    
    file->flags = flags;
    file->is_open = 1;
    file->position = (flags & FAT32_O_APPEND) ? file->file_size : 0;
    
    return 0;
}


int fat32_close(fat32_file_t* file) {
    if (!file || !file->is_open)
        return -1;
    
    if (fat32_read_cluster(file->dir_cluster, cluster_buffer) != 0)
        return -1;
    
    fat32_dirent_t* entry = (fat32_dirent_t*)(cluster_buffer + file->dir_entry_offset);
    entry->file_size = file->file_size;
    entry->first_cluster_low = file->first_cluster & 0xFFFF;
    entry->first_cluster_high = (file->first_cluster >> 16) & 0xFFFF;
    
    fat32_write_cluster(file->dir_cluster, cluster_buffer);
    
    file->is_open = 0;
    return 0;
}


int fat32_read(fat32_file_t* file, void* buffer, uint32_t size) {
    if (!file || !file->is_open || !buffer)
        return -1;
    
    if (file->position >= file->file_size)
        return 0;
    
    if (file->position + size > file->file_size)
        size = file->file_size - file->position;
    
    uint32_t bytes_read = 0;
    uint32_t cluster = file->first_cluster;
    uint32_t skip_clusters = file->position / g_fs.bytes_per_cluster;
    
    for (uint32_t i = 0; i < skip_clusters && cluster >= 2; i++)
        cluster = fat32_read_fat(cluster);
    
    file->current_cluster = cluster;
    uint32_t offset_in_cluster = file->position % g_fs.bytes_per_cluster;
    
    while (size > 0 && cluster >= 2 && cluster < FAT32_EOC_MIN) {
        if (fat32_read_cluster(cluster, cluster_buffer) != 0)
            break;
        
        uint32_t to_copy = g_fs.bytes_per_cluster - offset_in_cluster;
        if (to_copy > size)
            to_copy = size;
        
        memcpy((uint8_t*)buffer + bytes_read, cluster_buffer + offset_in_cluster, to_copy);
        
        bytes_read += to_copy;
        size -= to_copy;
        file->position += to_copy;
        offset_in_cluster = 0;
        
        if (size > 0)
            cluster = fat32_read_fat(cluster);
    }
    
    return bytes_read;
}


int fat32_write(fat32_file_t* file, const void* buffer, uint32_t size) {
    if (!file || !file->is_open || !buffer)
        return -1;
    
    if (file->first_cluster == 0) {
        file->first_cluster = fat32_alloc_cluster();
        if (file->first_cluster == 0)
            return -1;
        file->current_cluster = file->first_cluster;
    }
    
    uint32_t bytes_written = 0;
    uint32_t cluster = file->first_cluster;
    uint32_t skip_clusters = file->position / g_fs.bytes_per_cluster;
    
    for (uint32_t i = 0; i < skip_clusters && cluster >= 2; i++) {
        uint32_t next = fat32_read_fat(cluster);
        if (next >= FAT32_EOC_MIN) {
            next = fat32_alloc_cluster();
            if (next == 0)
                return bytes_written;
            fat32_write_fat(cluster, next);
        }
        cluster = next;
    }
    
    file->current_cluster = cluster;
    uint32_t offset_in_cluster = file->position % g_fs.bytes_per_cluster;
    
    while (size > 0 && cluster >= 2) {
        if (offset_in_cluster > 0 || size < g_fs.bytes_per_cluster) {
            if (fat32_read_cluster(cluster, cluster_buffer) != 0)
                break;
        }
        
        uint32_t to_copy = g_fs.bytes_per_cluster - offset_in_cluster;
        if (to_copy > size)
            to_copy = size;
        
        memcpy(cluster_buffer + offset_in_cluster, (uint8_t*)buffer + bytes_written, to_copy);
        
        if (fat32_write_cluster(cluster, cluster_buffer) != 0)
            break;
        
        bytes_written += to_copy;
        size -= to_copy;
        file->position += to_copy;
        
        if (file->position > file->file_size)
            file->file_size = file->position;
        
        offset_in_cluster = 0;
        
        if (size > 0) {
            uint32_t next = fat32_read_fat(cluster);
            if (next >= FAT32_EOC_MIN) {
                next = fat32_alloc_cluster();
                if (next == 0)
                    break;
                fat32_write_fat(cluster, next);
            }
            cluster = next;
        }
    }
    
    return bytes_written;
}


int fat32_seek(fat32_file_t* file, int32_t offset, uint8_t whence) {
    if (!file || !file->is_open)
        return -1;
    
    uint32_t new_pos;
    
    switch (whence) {
        case FAT32_SEEK_SET:
            new_pos = offset;
            break;
        case FAT32_SEEK_CUR:
            new_pos = file->position + offset;
            break;
        case FAT32_SEEK_END:
            new_pos = file->file_size + offset;
            break;
        default:
            return -1;
    }
    
    file->position = new_pos;
    return 0;
}


int fat32_tell(fat32_file_t* file) {
    if (!file || !file->is_open)
        return -1;
    return file->position;
}


int fat32_mkdir(const char* path) {
    uint32_t new_cluster = fat32_alloc_cluster();
    if (new_cluster == 0)
        return -1;
    
    if (fat32_create_entry(g_fs.root_dir_first_cluster, path, 
                          FAT32_ATTR_DIRECTORY, new_cluster) != 0) {
        fat32_free_cluster_chain(new_cluster);
        return -1;
    }
    
    memset(cluster_buffer, 0, g_fs.bytes_per_cluster);
    fat32_write_cluster(new_cluster, cluster_buffer);
    
    return 0;
}


int fat32_unlink(const char* path) {
    fat32_dirent_t entry;
    uint32_t cluster, offset;
    
    if (fat32_find_entry(g_fs.root_dir_first_cluster, path, &entry, &cluster, &offset) != 0)
        return -1;
    
    uint32_t first_cluster = ((uint32_t)entry.first_cluster_high << 16) | 
                            entry.first_cluster_low;
    
    if (first_cluster >= 2)
        fat32_free_cluster_chain(first_cluster);
    
    if (fat32_read_cluster(cluster, cluster_buffer) != 0)
        return -1;
    
    fat32_dirent_t* dir_entry = (fat32_dirent_t*)(cluster_buffer + offset);
    dir_entry->name[0] = 0xE5;
    
    return fat32_write_cluster(cluster, cluster_buffer);
}


int fat32_readdir(uint32_t cluster, uint32_t* index, fat32_file_info_t* info) {
    if (!info || !index)
        return -1;
    
    uint32_t current_cluster = cluster;
    uint32_t entries_per_cluster = g_fs.bytes_per_cluster / sizeof(fat32_dirent_t);
    uint32_t skip_clusters = (*index) / entries_per_cluster;
    uint32_t entry_in_cluster = (*index) % entries_per_cluster;
    
    for (uint32_t i = 0; i < skip_clusters && current_cluster >= 2; i++)
        current_cluster = fat32_read_fat(current_cluster);
    
    while (current_cluster >= 2 && current_cluster < FAT32_EOC_MIN) {
        if (fat32_read_cluster(current_cluster, cluster_buffer) != 0)
            return -1;
        
        fat32_dirent_t* entries = (fat32_dirent_t*)cluster_buffer;
        
        for (uint32_t i = entry_in_cluster; i < entries_per_cluster; i++) {
            if (entries[i].name[0] == 0x00)
                return -1;
            
            if (entries[i].name[0] == 0xE5 || entries[i].attr == FAT32_ATTR_LFN)
                continue;
            
            for (int j = 0; j < 8 && entries[i].name[j] != ' '; j++)
                info->name[j] = entries[i].name[j];
            
            if (entries[i].name[8] != ' ') {
                int len = strlen(info->name);
                info->name[len++] = '.';
                for (int j = 8; j < 11 && entries[i].name[j] != ' '; j++)
                    info->name[len++] = entries[i].name[j];
                info->name[len] = '\0';
            }
            
            info->size = entries[i].file_size;
            info->attr = entries[i].attr;
            info->first_cluster = ((uint32_t)entries[i].first_cluster_high << 16) | 
                                 entries[i].first_cluster_low;
            
            (*index)++;
            return 0;
        }
        
        entry_in_cluster = 0;
        current_cluster = fat32_read_fat(current_cluster);
    }
    
    return -1;
}


void fat32_list_dir(const char* path) {
    term_puts("Directory listing: ");
    term_puts(path);
    term_puts("\n");
    
    uint32_t cluster = g_fs.root_dir_first_cluster;
    uint32_t index = 0;
    fat32_file_info_t info;
    
    while (fat32_readdir(cluster, &index, &info) == 0) {
        if (info.attr & FAT32_ATTR_DIRECTORY) {
            term_puts("<DIR> ");
        } else {
            term_puts("      ");
        }
        
        term_puts(info.name);
        term_puts(" (");
        char size_str[16];
        itoa(info.size, size_str, 10);
        term_puts(size_str);
        term_puts(" bytes)\n");
    }
}





void fat32_print_info(void) {
    term_puts("\nFAT32 Filesystem Information:\n");
    term_puts("Bytes per sector: ");
    char buf[16];
    itoa(g_fs.boot.bytes_per_sector, buf, 10);
    term_puts(buf);
    term_puts("\nSectors per cluster: ");
    itoa(g_fs.boot.sectors_per_cluster, buf, 10);
    term_puts(buf);
    term_puts("\nReserved sectors: ");
    itoa(g_fs.boot.reserved_sectors, buf, 10);
    term_puts(buf);
    term_puts("\nNumber of FATs: ");
    itoa(g_fs.boot.num_fats, buf, 10);
    term_puts(buf);
    term_puts("\nFAT size: ");
    itoa(g_fs.boot.fat_size_32, buf, 10);
    term_puts(buf);
    term_puts(" sectors\nRoot cluster: ");
    itoa(g_fs.boot.root_cluster, buf, 10);
    term_puts(buf);
    term_puts("\nTotal clusters: ");
    itoa(g_fs.total_clusters, buf, 10);
    term_puts(buf);
    term_puts("\nFree clusters: ");
    itoa(g_fs.fsinfo.free_count, buf, 10);
    term_puts(buf);
    term_puts("\n\n");
}

uint32_t fat32_get_root_cluster(void) {
    return g_fs.root_dir_first_cluster;
}
