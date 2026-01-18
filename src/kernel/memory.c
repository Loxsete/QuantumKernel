#include "kernel/memory.h"
#include "lib/string.h"

#define HEAP_START 0x20000000
#define HEAP_SIZE  0x10000000

static uint32_t heap_current = HEAP_START;

typedef struct mem_block {
    uint32_t size;
    int free;
    struct mem_block *next;
} mem_block_t;

static mem_block_t *heap_list = 0;

void memory_init(uint32_t mem_size) {
    (void)mem_size;
    
    heap_list = (mem_block_t*)heap_current;
    heap_list->size = HEAP_SIZE - sizeof(mem_block_t);
    heap_list->free = 1;
    heap_list->next = 0;
}



void *kmalloc_aligned(uint32_t size, uint32_t align) {
    uint32_t addr = (uint32_t)kmalloc(size + align);
    if (!addr) return 0;
    
    uint32_t aligned = (addr + align - 1) & ~(align - 1);
    return (void*)aligned;
}

void kfree(void *ptr) {
    if (!ptr) return;
    
    mem_block_t *block = (mem_block_t*)((uint32_t)ptr - sizeof(mem_block_t));
    block->free = 1;
    
    mem_block_t *current = heap_list;
    while (current && current->next) {
        if (current->free && current->next->free) {
            current->size += sizeof(mem_block_t) + current->next->size;
            current->next = current->next->next;
        } else {
            current = current->next;
        }
    }
}

uint32_t virt_to_phys(void *virt) {
    return (uint32_t)virt;
}

void *phys_to_virt(uint32_t phys) {
    return (void*)phys;
}
