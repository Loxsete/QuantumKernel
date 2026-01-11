bits 32
global context_switch

context_switch:
    mov [eax], esp
    mov [eax+4], ebp
    mov [eax+8], ecx

    mov esp, [edx]
    mov ebp, [edx+4]
    mov ecx, [edx+8]

    jmp ecx
