#include "kernel/elf.h"
#include "kernel/task.h"
#include "kernel/memory.h"
#include "cpu/paging.h"
#include "fs/fat32.h"
#include "lib/libc.h"
#include "lib/string.h"
#include "drivers/terminal.h"

#define USER_STACK_SIZE 0x4000
#define USER_STACK_TOP 0xC0000000

static int load_elf(const char* path, uint32_t* entry_point, page_context_t* ctx) {
    fat32_file_t file;
    if (fat32_open(&file, path, 0) != 0) {
        term_puts("exec: failed to open file\n");
        return -1;
    }
    
    elf_header_t header;
    if (fat32_read(&file, &header, sizeof(header)) != sizeof(header)) {
        term_puts("exec: failed to read ELF header\n");
        fat32_close(&file);
        return -1;
    }
    
    if (header.magic != ELF_MAGIC || 
        header.class != 1 ||
        header.machine != 3) {
        term_puts("exec: invalid ELF file\n");
        fat32_close(&file);
        return -1;
    }
    
    *entry_point = header.entry;
    
    
    for (uint16_t i = 0; i < header.phnum; i++) {
        elf_program_header_t ph;
        
        fat32_seek(&file, header.phoff + i * header.phentsize, FAT32_SEEK_SET);
        if (fat32_read(&file, &ph, sizeof(ph)) != sizeof(ph)) {
            fat32_close(&file);
            return -1;
        }
        
        if (ph.type != PT_LOAD) {
            continue;
        }
        
        
        uint32_t page_count = (ph.memsz + 0xFFF) / 0x1000;
        
        
        uint32_t phys_base = (uint32_t)kmalloc_aligned(page_count * 0x1000, 0x1000);
        if (!phys_base) {
            term_puts("exec: failed to allocate memory\n");
            fat32_close(&file);
            return -1;
        }
        
        
        memset((void*)phys_base, 0, page_count * 0x1000);
        
        
        if (ph.filesz > 0) {
            fat32_seek(&file, ph.offset, FAT32_SEEK_SET);
            uint32_t phys_offset = ph.vaddr & 0xFFF;  
            fat32_read(&file, (void*)(phys_base + phys_offset), ph.filesz);
        }
        
        
        for (uint32_t p = 0; p < page_count; p++) {
            uint32_t vaddr = (ph.vaddr & 0xFFFFF000) + p * 0x1000;
            uint32_t phys = phys_base + p * 0x1000;
            paging_map_page(ctx, vaddr, phys, PAGE_PRESENT | PAGE_WRITE | PAGE_USER);
        }
    }
    
    fat32_close(&file);
    return 0;
}

int exec(const char* path) {
    term_puts("exec: loading ");
    term_puts(path);
    term_puts("\n");
    
    
    page_context_t* new_ctx = paging_create_context();
    if (!new_ctx) {
        term_puts("exec: failed to create page context\n");
        return -1;
    }
    
    
    uint32_t entry_point;
    if (load_elf(path, &entry_point, new_ctx) != 0) {
        term_puts("exec: failed to load ELF\n");
        return -1;
    }
    
    
    uint32_t stack_phys = (uint32_t)kmalloc_aligned(USER_STACK_SIZE, 0x1000);
    if (!stack_phys) {
        term_puts("exec: failed to allocate stack\n");
        return -1;
    }
    
    memset((void*)stack_phys, 0, USER_STACK_SIZE);
    
    
    for (uint32_t i = 0; i < USER_STACK_SIZE; i += 0x1000) {
        paging_map_page(new_ctx, 
                       USER_STACK_TOP - USER_STACK_SIZE + i,
                       stack_phys + i,
                       PAGE_PRESENT | PAGE_WRITE | PAGE_USER);
    }
    
    
    task_t* current = task_current();
    current->page_ctx = new_ctx;
    paging_switch_context(new_ctx);
    
    term_puts("exec: jumping to entry point 0x");
    char buf[16];
    itoa(entry_point, buf, 16);
    term_puts(buf);
    term_puts("\n");
    
    
    asm volatile(
        "mov $0x23, %%ax\n"        
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"
        
        "pushl $0x23\n"            
        "pushl %0\n"               
        "pushfl\n"                 
        "popl %%eax\n"
        "orl $0x200, %%eax\n"      
        "pushl %%eax\n"
        "pushl $0x1B\n"            
        "pushl %1\n"               
        "iret\n"
        :
        : "r"(USER_STACK_TOP - 4), "r"(entry_point)
        : "eax"
    );
    
    __builtin_unreachable();
}
