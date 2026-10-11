bits 32
section .text

global IRQ0, IRQ1
extern IRQ_handler          ; void IRQ_handler(int n)

IRQ0:
    push dword 32           ; vector number
    jmp IRQ_common

IRQ1:
    push dword 33
    jmp IRQ_common

IRQ_common:
    pusha                   ; save all registers
    push dword [esp + 32]   ; copy vector number as C argument
    call IRQ_handler
    add esp, 4
    popa                    ; restore registers
    add esp, 4              ; drop vector number
    iret                    ; return from interrupt