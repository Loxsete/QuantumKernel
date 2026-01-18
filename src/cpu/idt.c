#include "cpu/idt.h"
#include "drivers/terminal.h"
#include "lib/libc.h"
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

extern void isr0(void);
extern void isr1(void);
extern void isr2(void);
extern void isr3(void);
extern void isr4(void);
extern void isr5(void);
extern void isr6(void);
extern void isr7(void);
extern void isr8(void);
extern void isr9(void);
extern void isr10(void);
extern void isr11(void);
extern void isr12(void);
extern void isr13(void);
extern void isr14(void);
extern void isr15(void);
extern void isr16(void);
extern void isr17(void);
extern void isr18(void);
extern void isr19(void);
extern void isr20(void);
extern void isr21(void);
extern void isr22(void);
extern void isr23(void);
extern void isr24(void);
extern void isr25(void);
extern void isr26(void);
extern void isr27(void);
extern void isr28(void);
extern void isr29(void);
extern void isr30(void);
extern void isr31(void);

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
    
    idt_set_gate(0, (uint32_t)isr0, 0x08, 0x8E);
    idt_set_gate(1, (uint32_t)isr1, 0x08, 0x8E);
    idt_set_gate(2, (uint32_t)isr2, 0x08, 0x8E);
    idt_set_gate(3, (uint32_t)isr3, 0x08, 0x8E);
    idt_set_gate(4, (uint32_t)isr4, 0x08, 0x8E);
    idt_set_gate(5, (uint32_t)isr5, 0x08, 0x8E);
    idt_set_gate(6, (uint32_t)isr6, 0x08, 0x8E);
    idt_set_gate(7, (uint32_t)isr7, 0x08, 0x8E);
    idt_set_gate(8, (uint32_t)isr8, 0x08, 0x8E);
    idt_set_gate(9, (uint32_t)isr9, 0x08, 0x8E);
    idt_set_gate(10, (uint32_t)isr10, 0x08, 0x8E);
    idt_set_gate(11, (uint32_t)isr11, 0x08, 0x8E);
    idt_set_gate(12, (uint32_t)isr12, 0x08, 0x8E);
    idt_set_gate(13, (uint32_t)isr13, 0x08, 0x8E);
    idt_set_gate(14, (uint32_t)isr14, 0x08, 0x8E);
    idt_set_gate(15, (uint32_t)isr15, 0x08, 0x8E);
    idt_set_gate(16, (uint32_t)isr16, 0x08, 0x8E);
    idt_set_gate(17, (uint32_t)isr17, 0x08, 0x8E);
    idt_set_gate(18, (uint32_t)isr18, 0x08, 0x8E);
    idt_set_gate(19, (uint32_t)isr19, 0x08, 0x8E);
    idt_set_gate(20, (uint32_t)isr20, 0x08, 0x8E);
    idt_set_gate(21, (uint32_t)isr21, 0x08, 0x8E);
    idt_set_gate(22, (uint32_t)isr22, 0x08, 0x8E);
    idt_set_gate(23, (uint32_t)isr23, 0x08, 0x8E);
    idt_set_gate(24, (uint32_t)isr24, 0x08, 0x8E);
    idt_set_gate(25, (uint32_t)isr25, 0x08, 0x8E);
    idt_set_gate(26, (uint32_t)isr26, 0x08, 0x8E);
    idt_set_gate(27, (uint32_t)isr27, 0x08, 0x8E);
    idt_set_gate(28, (uint32_t)isr28, 0x08, 0x8E);
    idt_set_gate(29, (uint32_t)isr29, 0x08, 0x8E);
    idt_set_gate(30, (uint32_t)isr30, 0x08, 0x8E);
    idt_set_gate(31, (uint32_t)isr31, 0x08, 0x8E);
    
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

void isr_handler(uint32_t int_no, uint32_t err_code, uint32_t eip, uint32_t cs, uint32_t eflags) {
    (void)cs;
    (void)eflags;
    
    term_puts("\n!!! EXCEPTION ");
    char buf[16];
    itoa(int_no, buf, 10);
    term_puts(buf);
    term_puts(" !!!\n");
    
    if (int_no == 14) {
        uint32_t cr2;
        asm volatile("mov %%cr2, %0" : "=r"(cr2));
        term_puts("Page Fault at: 0x");
        itoa(cr2, buf, 16);
        term_puts(buf);
        term_puts("\n");
    }
    
    term_puts("EIP: 0x");
    itoa(eip, buf, 16);
    term_puts(buf);
    term_puts(" Error: 0x");
    itoa(err_code, buf, 16);
    term_puts(buf);
    term_puts("\n");
    
    asm volatile("cli; hlt");
    for(;;);
}
