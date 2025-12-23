bits 32
global enter_user
extern user_entry
extern USER_STACK_TOP

enter_user:
    cli
    mov ax, 0x23        ; user data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, USER_STACK_TOP

    push 0x23           ; SS
    push USER_STACK_TOP ; ESP
    pushf               ; EFLAGS
    push 0x1b           ; CS
    push user_entry     ; EIP
    iret
