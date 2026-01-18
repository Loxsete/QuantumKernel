#include "cpu/paging.h"
#include "kernel/memory.h"
#include "lib/libc.h"

page_context_t *kernel_page_context = 0;
static page_context_t *current_context = 0;

static uint32_t page_directory_physical[PAGE_ENTRIES] __attribute__((aligned(4096)));
static uint32_t page_tables_physical[768][PAGE_ENTRIES] __attribute__((aligned(4096)));

void paging_init(void) {
    kernel_page_context = kmalloc(sizeof(page_context_t));
    
    kernel_page_context->directory = (page_directory_t*)page_directory_physical;
    kernel_page_context->tables = kmalloc(sizeof(page_table_t*) * PAGE_ENTRIES);
    kernel_page_context->physical_addr = (uint32_t)page_directory_physical;
    
    memset((void*)kernel_page_context->directory, 0, sizeof(page_directory_t));
    memset((void*)kernel_page_context->tables, 0, sizeof(page_table_t*) * PAGE_ENTRIES);
    
    for (int i = 0; i < 512; i++) {
        kernel_page_context->tables[i] = (page_table_t*)(&page_tables_physical[i]);
        memset((void*)kernel_page_context->tables[i], 0, sizeof(page_table_t));
        
        (*kernel_page_context->directory)[i] = 
            ((uint32_t)kernel_page_context->tables[i]) | PAGE_PRESENT | PAGE_WRITE | PAGE_USER;
    }
    
    for (uint32_t i = 0; i < 0x80000000; i += PAGE_SIZE) {
        uint32_t pd_index = i >> 22;
        uint32_t pt_index = (i >> 12) & 0x3FF;
        
        if (pd_index < 512) {
            (*kernel_page_context->tables[pd_index])[pt_index] = i | PAGE_PRESENT | PAGE_WRITE | PAGE_USER;
        }
    }
    
    for (int i = 960; i < 1024; i++) {
        kernel_page_context->tables[i] = (page_table_t*)(&page_tables_physical[512 + (i - 960)]);
        memset((void*)kernel_page_context->tables[i], 0, sizeof(page_table_t));
        
        (*kernel_page_context->directory)[i] = 
            ((uint32_t)kernel_page_context->tables[i]) | PAGE_PRESENT | PAGE_WRITE | PAGE_USER;
    }
    
    for (uint32_t i = 0xC0000000; i != 0; i += PAGE_SIZE) {
        uint32_t pd_index = i >> 22;
        uint32_t pt_index = (i >> 12) & 0x3FF;
        
        (*kernel_page_context->tables[pd_index])[pt_index] = i | PAGE_PRESENT | PAGE_WRITE | PAGE_USER;
    }
    
    current_context = kernel_page_context;
    paging_enable();
}

void paging_enable(void) {
    asm volatile(
        "mov %0, %%cr3\n"
        "mov %%cr0, %%eax\n"
        "or $0x80000000, %%eax\n"
        "mov %%eax, %%cr0\n"
        : : "r"(kernel_page_context->physical_addr)
        : "eax"
    );
}

page_context_t *paging_create_context(void) {
    return kernel_page_context;
}

void paging_switch_context(page_context_t *ctx) {
    if (ctx == current_context) return;
    
    current_context = ctx;
    asm volatile("mov %0, %%cr3" : : "r"(ctx->physical_addr));
}

void paging_map_page(page_context_t *ctx, uint32_t virt, uint32_t phys, uint32_t flags) {
    uint32_t pd_index = virt >> 22;
    uint32_t pt_index = (virt >> 12) & 0x3FF;
    
    if (!ctx->tables[pd_index]) {
        ctx->tables[pd_index] = kmalloc_aligned(sizeof(page_table_t), 4096);
        memset((void*)ctx->tables[pd_index], 0, sizeof(page_table_t));
        
        uint32_t pt_phys = (uint32_t)virt_to_phys(ctx->tables[pd_index]);
        (*ctx->directory)[pd_index] = pt_phys | PAGE_PRESENT | PAGE_WRITE | PAGE_USER;
    }
    
    (*ctx->tables[pd_index])[pt_index] = (phys & 0xFFFFF000) | flags;
    
    asm volatile("invlpg (%0)" : : "r"(virt) : "memory");
}

void paging_unmap_page(page_context_t *ctx, uint32_t virt) {
    uint32_t pd_index = virt >> 22;
    uint32_t pt_index = (virt >> 12) & 0x3FF;
    
    if (ctx->tables[pd_index]) {
        (*ctx->tables[pd_index])[pt_index] = 0;
        asm volatile("invlpg (%0)" : : "r"(virt) : "memory");
    }
}

uint32_t paging_get_physical(page_context_t *ctx, uint32_t virt) {
    uint32_t pd_index = virt >> 22;
    uint32_t pt_index = (virt >> 12) & 0x3FF;
    uint32_t offset = virt & 0xFFF;
    
    if (!ctx->tables[pd_index]) return 0;
    
    uint32_t page = (*ctx->tables[pd_index])[pt_index];
    if (!(page & PAGE_PRESENT)) return 0;
    
    return (page & 0xFFFFF000) | offset;
}
