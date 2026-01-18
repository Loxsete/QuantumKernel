#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>

#define PAGE_SIZE 4096
#define PAGE_ENTRIES 1024

#define PAGE_PRESENT    0x1
#define PAGE_WRITE      0x2
#define PAGE_USER       0x4
#define PAGE_ACCESSED   0x20
#define PAGE_DIRTY      0x40

typedef uint32_t page_entry_t;
typedef uint32_t page_table_t[PAGE_ENTRIES];
typedef uint32_t page_directory_t[PAGE_ENTRIES];

typedef struct {
    page_directory_t *directory;
    page_table_t **tables;
    uint32_t physical_addr;
} page_context_t;

void paging_init(void);
page_context_t *paging_create_context(void);
void paging_switch_context(page_context_t *ctx);
void paging_map_page(page_context_t *ctx, uint32_t virt, uint32_t phys, uint32_t flags);
void paging_unmap_page(page_context_t *ctx, uint32_t virt);
uint32_t paging_get_physical(page_context_t *ctx, uint32_t virt);
void paging_enable(void);

extern page_context_t *kernel_page_context;

#endif
