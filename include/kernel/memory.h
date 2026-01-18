#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

void memory_init(uint32_t mem_size);
void *kmalloc(uint32_t size);
void *kmalloc_aligned(uint32_t size, uint32_t align);
void kfree(void *ptr);
uint32_t virt_to_phys(void *virt);
void *phys_to_virt(uint32_t phys);

#endif
