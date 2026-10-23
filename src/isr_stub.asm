bits 32
section .text

global IRQ0, IRQ1
extern IRQ_handler

IRQ0:
    push dword 32 
    jmp IRQ_common

IRQ1:
    push dword 33
    jmp IRQ_common

IRQ_common:
    pusha
    push dword [esp + 32]
    call IRQ_handler
    add esp, 4
    popa
    add esp, 4
    iret