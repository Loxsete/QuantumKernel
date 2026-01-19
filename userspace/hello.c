#include "libc/include/stdio.h"
#include "libc/include/syscall.h"
#include "libc/include/string.h"

void _start(void) {
    printf("Hello from userspace!\n");
    printf("This is a test program.\n");
    printf("Number: %d\n", 42);
    printf("Hex: 0x%x\n", 0xDEADBEEF);
    
    
    char buffer[64];
    strcpy(buffer, "String test: ");
    strcat(buffer, "success!");
    puts(buffer);
    
    
    printf("Sleeping for 1 second...\n");
    sleep_ms(1000);
    printf("Done!\n");
    
    exit();
}
