#include "fs/vfs.h"
#include "fs/fat32.h"
#include "fs/procfs.h"
#include "lib/string.h"

static vfs_ops_t* proc_ops;
static vfs_ops_t* fat_ops;

void vfs_init(void) {
    proc_ops = procfs_get_ops();
    fat_ops = fat32_get_ops();
}

static vfs_ops_t* select_fs(const char* path) {
    if (!strncmp(path, "/proc", 5))
        return proc_ops;
    return fat_ops;
}

int vfs_open(const char* path, vfs_file_t* out, uint32_t flags) {
    vfs_ops_t* fs = select_fs(path);
    return fs->open(path, out, flags);
}

int vfs_read(vfs_file_t* file, void* buf, uint32_t size) {
    return file->ops->read(file, buf, size);
}

int vfs_write(vfs_file_t* file, const void* buf, uint32_t size) {
    return file->ops->write(file, buf, size);
}

int vfs_close(vfs_file_t* file) {
    return file->ops->close(file);
}

