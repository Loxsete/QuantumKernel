#ifndef VFS_H
#define VFS_H

#include <stdint.h>

struct vfs_ops;
typedef struct vfs_ops vfs_ops_t;

typedef struct vfs_file {
    void* fs_data;
    vfs_ops_t* ops;   
    uint32_t pos;
    uint32_t size;
    uint32_t flags;
} vfs_file_t;

struct vfs_ops {
    int (*open)(const char* path, vfs_file_t* out, uint32_t flags);
    int (*read)(vfs_file_t* file, void* buf, uint32_t size);
    int (*write)(vfs_file_t* file, const void* buf, uint32_t size);
    int (*close)(vfs_file_t* file);
    int (*readdir)(uint32_t* index, void* info);
};

void vfs_init(void);
int vfs_open(const char* path, vfs_file_t* out, uint32_t flags);
int vfs_read(vfs_file_t* file, void* buf, uint32_t size);
int vfs_write(vfs_file_t* file, const void* buf, uint32_t size);
int vfs_close(vfs_file_t* file);

#endif
