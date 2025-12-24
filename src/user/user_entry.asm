bits 32

global enter_user
extern user_main

enter_user:
    cli
    
    mov ax, 0x23
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    push 0x23
    push 0x9FFFF0
    
    pushf
    pop eax
    or eax, 0x200
    push eax
    
    push 0x1B
    push user_main
    
    iret
