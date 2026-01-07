#ifndef PROCFS_H
#define PROCFS_H

#include <stdint.h>
#include "fs/vfs.h"

vfs_ops_t* procfs_get_ops(void);

#endif
