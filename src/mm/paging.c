#include <stdint.h>

#define PAGE_PRESENT  0x1
#define PAGE_RW       0x2

// page directory + table 
__attribute__((aligned(4096)))
uint32_t page_directory[1024];

__attribute__((aligned(4096)))
uint32_t first_page_table[1024];

void paging_init(void) {
    for (int i = 0; i < 1024; i++) {
        first_page_table[i] = (i * 0x1000) | PAGE_PRESENT | PAGE_RW;
    }

    page_directory[0] = ((uint32_t)first_page_table) | PAGE_PRESENT | PAGE_RW;

    for (int i = 1; i < 1024; i++) {
        page_directory[i] = 0;
    }

    asm volatile("mov %0, %%cr3" :: "r"(page_directory));

    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    asm volatile("mov %0, %%cr0" :: "r"(cr0));
}
