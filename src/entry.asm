bits 32

section .multiboot
align 4
dd 0x1BADB002          ; Multiboot magic number
dd 0x0                 ; Flags (we don't need anything special)
dd -(0x1BADB002 + 0x0) ; Checksum

section .bss
align 16
stack_bottom:
    resb 16384
stack_top:
    
global stack_top


section .text
global _start
extern kernel_main   ; We will Define this function in C

_start:
 
 mov esp, stack_top  ; Set up stack
 call kernel_main    ; Call kernel main function
 hlt                 ; Halt CPU (loop forever)

global syscall_handler
extern syscall_dispatch

syscall_handler:
    pusha

    push ds
    push es
    push fs
    push gs

    mov ax, 0x10     ; kernel data segment
    mov ds, ax
    mov es, ax

    push esp
    call syscall_dispatch
    add esp, 4

    pop gs
    pop fs
    pop es
    pop ds

    popa
    iret


extern syscall_invoke


