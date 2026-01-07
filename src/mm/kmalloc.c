#include "mm/kmalloc.h"

#define PAGE_SIZE 4096
static uintptr_t heap_ptr = 0x01000000; 

static inline uintptr_t align_up(uintptr_t addr, size_t align) {
    return (addr + align - 1) & ~(align - 1);
}

void *kmalloc(size_t size) {
    void *addr = (void *)heap_ptr;
    heap_ptr += size;
    return addr;
}

void *kmalloc_a(size_t size) {
    heap_ptr = align_up(heap_ptr, PAGE_SIZE);
    void *addr = (void *)heap_ptr;
    heap_ptr += size;
    return addr;
}
