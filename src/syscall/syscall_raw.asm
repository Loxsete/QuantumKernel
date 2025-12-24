bits 32

global syscall_handler
extern syscall_dispatch

syscall_handler:
    push ds
    push es
    push fs
    push gs
    pushad
    
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    mov eax, esp
    push eax
    call syscall_dispatch
    add esp, 4
    
    popad
    pop gs
    pop fs
    pop es
    pop ds
    
    iret

global syscall_invoke
syscall_invoke:
    mov eax, [esp+4]
    mov ebx, [esp+8]
    mov ecx, [esp+12]
    mov edx, [esp+16]
    int 0x80
    ret
