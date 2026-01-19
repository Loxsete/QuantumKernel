#include "lib/libc.h"
#include <stddef.h>
#include <stdint.h>

static uint8_t kernel_heap[1024*1024]; 
static void* heap = kernel_heap;

void* malloc(size_t size) {
    void* ptr = heap;
    heap = (uint8_t*)heap + size;
    return ptr;
}

void free(void* ptr) {
    (void)ptr;
}
