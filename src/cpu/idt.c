#include "cpu/idt.h"
#include <stdint.h>

#define IDT_SIZE 256

static idt_entry_t idt[IDT_SIZE];
static idt_ptr_t idt_ptr;

extern void irq0_handler(void);
extern void irq1_handler(void);
extern void irq2_handler(void);
extern void irq3_handler(void);
extern void irq4_handler(void);
extern void irq5_handler(void);
extern void irq6_handler(void);
extern void irq7_handler(void);
extern void irq8_handler(void);
extern void irq9_handler(void);
extern void irq10_handler(void);
extern void irq11_handler(void);
extern void irq12_handler(void);
extern void irq13_handler(void);
extern void irq14_handler(void);
extern void irq15_handler(void);

void idt_set_gate(uint8_t num, uint32_t handler, uint16_t selector, uint8_t type_attr) {
    idt[num].offset_low  = handler & 0xFFFF;
    idt[num].selector    = selector;
    idt[num].zero        = 0;
    idt[num].type_attr   = type_attr;
    idt[num].offset_high = (handler >> 16) & 0xFFFF;
}

void idt_init(void) {
    idt_ptr.limit = sizeof(idt) - 1;
    idt_ptr.base  = (uint32_t)&idt;
    
    for (int i = 0; i < IDT_SIZE; i++) {
        idt_set_gate(i, 0, 0, 0);
    }
    
    idt_set_gate(32, (uint32_t)irq0_handler, 0x08, 0x8E);
    idt_set_gate(33, (uint32_t)irq1_handler, 0x08, 0x8E);
    idt_set_gate(34, (uint32_t)irq2_handler, 0x08, 0x8E);
    idt_set_gate(35, (uint32_t)irq3_handler, 0x08, 0x8E);
    idt_set_gate(36, (uint32_t)irq4_handler, 0x08, 0x8E);
    idt_set_gate(37, (uint32_t)irq5_handler, 0x08, 0x8E);
    idt_set_gate(38, (uint32_t)irq6_handler, 0x08, 0x8E);
    idt_set_gate(39, (uint32_t)irq7_handler, 0x08, 0x8E);
    idt_set_gate(40, (uint32_t)irq8_handler, 0x08, 0x8E);
    idt_set_gate(41, (uint32_t)irq9_handler, 0x08, 0x8E);
    idt_set_gate(42, (uint32_t)irq10_handler, 0x08, 0x8E);
    idt_set_gate(43, (uint32_t)irq11_handler, 0x08, 0x8E);
    idt_set_gate(44, (uint32_t)irq12_handler, 0x08, 0x8E);
    idt_set_gate(45, (uint32_t)irq13_handler, 0x08, 0x8E);
    idt_set_gate(46, (uint32_t)irq14_handler, 0x08, 0x8E);
    idt_set_gate(47, (uint32_t)irq15_handler, 0x08, 0x8E);
    
    asm volatile ("lidt %0" : : "m"(idt_ptr));
}
