void _start(void) {
    const char* msg = "Hello from user program!\n";
    
    asm volatile(
        "mov $1, %%eax\n"      // SYS_WRITE
        "mov $1, %%ebx\n"      // fd = 1 (stdout)
        "mov %0, %%ecx\n"      // buffer
        "mov $26, %%edx\n"     // length
        "int $0x80\n"
        :
        : "r"(msg)
        : "eax", "ebx", "ecx", "edx"
    );
    
    asm volatile(
        "mov $3, %%eax\n"      // SYS_EXIT
        "int $0x80\n"
        :
        :
        : "eax"
    );
    
    while(1);
}
