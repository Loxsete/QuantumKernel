#include "cpu/tss.h"
#include "cpu/gdt.h"
#include <stdint.h>

struct tss_entry tss;
extern uint32_t stack_top;

void tss_init(void) {
    for (unsigned int i = 0; i < sizeof(struct tss_entry); i++)
        ((uint8_t*)&tss)[i] = 0;
    
    tss.ss0  = 0x10;
    tss.esp0 = (uint32_t)&stack_top;
    
    tss.cs = 0x0B;
    tss.ss = tss.ds = tss.es = tss.fs = tss.gs = 0x13;
    
    extern void gdt_set_tss(int num, uint32_t base, uint32_t limit);
    gdt_set_tss(5, (uint32_t)&tss, sizeof(struct tss_entry)-1);
    
    extern void tss_flush(void);
    tss_flush();
}
