bits 32
section .multiboot
align 4
MB_MAGIC equ 0x1BADB002
MB_FLAGS equ 0x00000007
MB_CHECKSUM equ -(MB_MAGIC + MB_FLAGS)
dd MB_MAGIC
dd MB_FLAGS
dd MB_CHECKSUM
dd 0
dd 0
dd 0
dd 0
dd 0
dd 0
dd 1024
dd 768
dd 32

section .bss
align 16
stack_bottom:
    resb 16384
stack_top:
global stack_top

section .text
global _start
extern kernel_main

_start:
    cli
    mov esp, stack_top
    push ebx
    push eax
    call kernel_main
.hang:
    hlt
    jmp .hang


