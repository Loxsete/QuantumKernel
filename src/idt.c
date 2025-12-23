// idt.c
#include "idt.h"

#define IDT_SIZE 256

static idt_entry_t idt[IDT_SIZE];
static idt_ptr_t idt_ptr;
extern void irq1_handler(void); 


void idt_set_gate(uint8_t num, uint32_t handler, uint16_t selector, uint8_t type_attr) {
    idt[num].offset_low  = handler & 0xFFFF;
    idt[num].selector    = selector;
    idt[num].zero        = 0;
    idt[num].type_attr   = type_attr;
    idt[num].offset_high = (handler >> 16) & 0xFFFF;
}

/*
 * Init IDT
 */
void idt_init(void) {
    idt_ptr.limit = sizeof(idt) - 1;
    idt_ptr.base  = (uint32_t)&idt;

    
    for (int i = 0; i < IDT_SIZE; i++) {
        idt_set_gate(i, 0, 0, 0);
    }

    
    asm volatile ("lidt %0" : : "m"(idt_ptr));

    idt_set_gate(
        33,                    // 32 + 1 (IRQ1)
        (uint32_t)irq1_handler,
        0x08,                  // kernel code segment
        0x8E                   // interrupt gate
    );

    asm volatile ("lidt %0" : : "m"(idt_ptr));
}
