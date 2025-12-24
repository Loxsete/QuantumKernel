bits 32

global irq0_handler
global irq1_handler
global irq2_handler
global irq3_handler
global irq4_handler
global irq5_handler
global irq6_handler
global irq7_handler
global irq8_handler
global irq9_handler
global irq10_handler
global irq11_handler
global irq12_handler
global irq13_handler
global irq14_handler
global irq15_handler

extern timer_callback
extern keyboard_irq

irq0_handler:
    pushad
    push ds
    push es
    push fs
    push gs
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    call timer_callback
    mov al, 0x20
    out 0x20, al
    pop gs
    pop fs
    pop es
    pop ds
    popad
    iret

irq1_handler:
    pushad
    push ds
    push es
    push fs
    push gs
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    call keyboard_irq
    mov al, 0x20
    out 0x20, al
    pop gs
    pop fs
    pop es
    pop ds
    popad
    iret

irq2_handler:
    pushad
    mov al, 0x20
    out 0x20, al
    popad
    iret

irq3_handler:
    pushad
    mov al, 0x20
    out 0x20, al
    popad
    iret

irq4_handler:
    pushad
    mov al, 0x20
    out 0x20, al
    popad
    iret

irq5_handler:
    pushad
    mov al, 0x20
    out 0x20, al
    popad
    iret

irq6_handler:
    pushad
    mov al, 0x20
    out 0x20, al
    popad
    iret

irq7_handler:
    pushad
    mov al, 0x20
    out 0x20, al
    popad
    iret

irq8_handler:
    pushad
    mov al, 0x20
    out 0xA0, al
    mov al, 0x20
    out 0x20, al
    popad
    iret

irq9_handler:
    pushad
    mov al, 0x20
    out 0xA0, al
    mov al, 0x20
    out 0x20, al
    popad
    iret

irq10_handler:
    pushad
    mov al, 0x20
    out 0xA0, al
    mov al, 0x20
    out 0x20, al
    popad
    iret

irq11_handler:
    pushad
    mov al, 0x20
    out 0xA0, al
    mov al, 0x20
    out 0x20, al
    popad
    iret

irq12_handler:
    pushad
    mov al, 0x20
    out 0xA0, al
    mov al, 0x20
    out 0x20, al
    popad
    iret

irq13_handler:
    pushad
    mov al, 0x20
    out 0xA0, al
    mov al, 0x20
    out 0x20, al
    popad
    iret

irq14_handler:
    pushad
    mov al, 0x20
    out 0xA0, al
    mov al, 0x20
    out 0x20, al
    popad
    iret

irq15_handler:
    pushad
    mov al, 0x20
    out 0xA0, al
    mov al, 0x20
    out 0x20, al
    popad
    iret
