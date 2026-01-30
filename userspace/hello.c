#include "libc/include/stdio.h"
#include "libc/include/syscall.h"
#include "libc/include/string.h"

void _start(void) {
    printf("Hello from userspace!\n");
    
    
    uint32_t cwd = get_cwd_cluster();
    printf("CWD cluster: %d\n", cwd);
    
    uint32_t index = 0;
    file_info_t entry;  
    
    printf("Directory listing:\n");
    
    
    while (1) {
        int ret = readdir(cwd, &index, &entry);
        if (ret < 0)
            break;
        
        printf(" - %s  (%d bytes)\n", entry.name, entry.size);
    }
    
    printf("Done listing.\n");
    sleep_ms(1000);
    exit();
}
