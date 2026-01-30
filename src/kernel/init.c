

#include "kernel/exec.h"
#include "drivers/terminal.h"
#include "kernel/task.h"
#include "lib/libc.h"

void init_process(void) {
    term_puts("Init: starting shell\n");
    
    
    int result = exec("shell.elf");
    
    
    term_puts("Init: exec failed with code ");
    char buf[16];
    itoa(result, buf, 10);
    term_puts(buf);
    term_puts("\n");
    
    
    for (;;) {
        asm volatile("hlt");
    }
}
