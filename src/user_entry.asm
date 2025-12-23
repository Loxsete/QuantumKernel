global enter_user
extern user_main

enter_user:
    cli

    mov ax, 0x23        ; user data
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push 0x23           ; SS
    push 0x90000        ; ESP
    push 0x202          ; EFLAGS
    push 0x1B           ; CS
    push user_main      ; EIP
    iret
