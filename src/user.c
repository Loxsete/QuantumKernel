#include "syscall.h"

void user_main(void) {
    write(1, "Hello from ring3 bro!\n", 18);
    exit();
}
