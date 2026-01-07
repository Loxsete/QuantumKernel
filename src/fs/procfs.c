#include "fs/procfs.h"
#include "drivers/timer.h"
#include "drivers/rtc.h"
#include "lib/string.h"
#include "lib/libc.h"

typedef struct {
    const char* name;
    int (*read)(char* buf, uint32_t size);
} proc_entry_t;

static int proc_uptime(char* buf, uint32_t size) {
    (void)size;

    uint32_t t = get_tick_count();
    itoa(t, buf, 10);
    strcat(buf, "\n");
    return strlen(buf);
}


static int proc_time(char* buf, uint32_t size) {
    (void)size;

    rtc_time_t t;
    rtc_time_sys(&t);
    char tmp[16];
    itoa(t.hour, tmp, 10);
    strcpy(buf, tmp);
    strcat(buf, ":");
    itoa(t.min, tmp, 10);
    strcat(buf, tmp);
    strcat(buf, "\n");
    return strlen(buf);
}


static proc_entry_t entries[] = {
    { "uptime", proc_uptime },
    { "time", proc_time },
};

static int proc_open(const char* path, vfs_file_t* out, uint32_t flags) {
    (void)flags;

    const char* name = path + 6;
    for (uint32_t i = 0; i < sizeof(entries)/sizeof(entries[0]); i++) {
        if (!strcmp(name, entries[i].name)) {
            out->fs_data = &entries[i];
            out->pos = 0;
            out->ops = procfs_get_ops();  
            out->flags = 0;
            return 0;
        }
    }
    return -1;
}


static int proc_read(vfs_file_t* file, void* buf, uint32_t size) {
    proc_entry_t* e = (proc_entry_t*)file->fs_data;
    char tmp[128];
    int len = e->read(tmp, sizeof(tmp));
    if (file->pos >= (uint32_t)len)
        return 0;
    int to_copy = len - file->pos;
    if (to_copy > (int)size)
        to_copy = size;
    memcpy(buf, tmp + file->pos, to_copy);
    file->pos += to_copy;
    return to_copy;
}

static int proc_write(vfs_file_t* file, const void* buf, uint32_t size) {
    (void)file;
    (void)buf;
    (void)size;
    return -1;
}


static int proc_close(vfs_file_t* file) {
    (void)file;
    return 0;
}


static vfs_ops_t ops = {
    .open = proc_open,
    .read = proc_read,
    .write = proc_write,
    .close = proc_close,
    .readdir = 0
};

vfs_ops_t* procfs_get_ops(void) {
    return &ops;
}
