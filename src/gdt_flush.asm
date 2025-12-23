bits 32
global gdt_flush
gdt_flush:
    lgdt [eax]
    mov ax, 0x10    ; kernel data
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:flush2
flush2:
    ret

global tss_flush
tss_flush:
    mov ax, 0x28    ; TSS сегмент в GDT
    ltr ax
    ret
