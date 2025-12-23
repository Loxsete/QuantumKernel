bits 32
global irq1_handler
extern keyboard_irq

irq1_handler:
	pusha
	call keyboard_irq
	
	mov al, 0x20
	out 0x20, al ; EOI PIC
	
	popa
	iret
