#include "fs/fat32.h"
#include "drivers/ata.h"
#include "drivers/terminal.h"
#include <string.h>
#include "lib/libc.h"

static fat32_boot_sector_t bpb;

static uint32_t fat_entry(uint32_t cluster) {
    uint32_t fat_sector = bpb.reserved_sector_count + (cluster * 4) / bpb.bytes_per_sector;
    uint8_t sector[512];
    ata_read_sector(fat_sector, sector);
    uint32_t* entries = (uint32_t*)sector;
    return entries[cluster % (bpb.bytes_per_sector / 4)] & 0x0FFFFFFF;
}

static uint32_t cluster_to_lba(uint32_t cluster) {
    return ((cluster - 2) * bpb.sectors_per_cluster) + bpb.reserved_sector_count + bpb.num_fats * bpb.fat_size_32;
}

int fat32_init(void) {
    uint8_t sector[512];
    if (ata_read_sector(0, sector) != 0) {
        term_puts("[FAT32] Failed to read boot sector\n");
        return -1;
    }
    bpb = *(fat32_boot_sector_t*)sector;
    term_puts("[FAT32] Initialized\n");
    return 0;
}

static int compare_filename(const char* name, fat32_dir_entry_t* entry) {
    char fname[12];
    memset(fname, ' ', 11);
    fname[11] = 0;

    int i = 0;
    for (; i < 11 && name[i] && name[i] != '.'; i++)
        fname[i] = name[i] >= 'a' && name[i] <= 'z' ? name[i]-32 : name[i]; // uppercase
    if (name[i] == '.') {
        int j = i + 1;
        for (int k = 8; k < 11 && name[j]; j++, k++)
            fname[k] = name[j] >= 'a' && name[j] <= 'z' ? name[j]-32 : name[j];
    }

    return memcmp(fname, entry->name, 11) == 0;
}

int fat32_read_file(const char* filename, uint8_t* buffer, uint32_t max_size) {
    uint32_t cluster = bpb.root_cluster;
    uint8_t sector[512];

    while (cluster < 0x0FFFFFF8) {
        for (uint8_t i = 0; i < bpb.sectors_per_cluster; i++) {
            if (ata_read_sector(cluster_to_lba(cluster) + i, sector) != 0)
                return -1;

            for (int offset = 0; offset < 512; offset += sizeof(fat32_dir_entry_t)) {
                fat32_dir_entry_t* entry = (fat32_dir_entry_t*)(sector + offset);
                if (entry->name[0] == 0) return -1; 
                if (entry->name[0] == 0xE5) continue;

                if (!(entry->attr & 0x0F) && compare_filename(filename, entry)) {
                    uint32_t file_cluster = ((uint32_t)entry->first_cluster_high << 16) | entry->first_cluster_low;
                    uint32_t remaining = entry->file_size;
                    uint32_t read_bytes = 0;

                    while (file_cluster < 0x0FFFFFF8 && remaining > 0 && read_bytes < max_size) {
                        for (uint8_t s = 0; s < bpb.sectors_per_cluster && remaining > 0; s++) {
                            ata_read_sector(cluster_to_lba(file_cluster) + s, sector);
                            uint32_t to_copy = remaining > 512 ? 512 : remaining;
                            if (read_bytes + to_copy > max_size) to_copy = max_size - read_bytes;
                            memcpy(buffer + read_bytes, sector, to_copy);
                            remaining -= to_copy;
                            read_bytes += to_copy;
                        }
                        file_cluster = fat_entry(file_cluster);
                    }
                    return read_bytes;
                }
            }
        }
        cluster = fat_entry(cluster);
    }
    return -1;
}

int fat32_write_file(const char* filename, const uint8_t* buffer, uint32_t size) {
    uint32_t cluster = bpb.root_cluster;
    uint8_t sector[512];

    while (cluster < 0x0FFFFFF8) {
        for (uint8_t i = 0; i < bpb.sectors_per_cluster; i++) {
            ata_read_sector(cluster_to_lba(cluster) + i, sector);

            for (int offset = 0; offset < 512; offset += sizeof(fat32_dir_entry_t)) {
                fat32_dir_entry_t* entry = (fat32_dir_entry_t*)(sector + offset);
                if (entry->name[0] == 0) return -1;
                if (entry->name[0] == 0xE5) continue;

                if (!(entry->attr & 0x0F) && compare_filename(filename, entry)) {
                    uint32_t file_cluster = ((uint32_t)entry->first_cluster_high << 16) | entry->first_cluster_low;
                    uint32_t remaining = size;
                    uint32_t written = 0;

                    while (file_cluster < 0x0FFFFFF8 && remaining > 0) {
                        for (uint8_t s = 0; s < bpb.sectors_per_cluster && remaining > 0; s++) {
                            uint32_t to_copy = remaining > 512 ? 512 : remaining;
                            memcpy(sector, buffer + written, to_copy);
                            ata_write_sector(cluster_to_lba(file_cluster) + s, sector);
                            remaining -= to_copy;
                            written += to_copy;
                        }
                        file_cluster = fat_entry(file_cluster);
                    }
                    entry->file_size = size;
                    return written;
                }
            }
        }
        cluster = fat_entry(cluster);
    }
    return -1;
}
