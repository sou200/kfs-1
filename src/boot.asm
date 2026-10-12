MBALIGN  equ 1
MEMINFO  equ 0
FLAGS    equ MBALIGN | MEMINFO
MAGIC    equ 0x1BADB002
CHECKSUM equ -(MAGIC + FLAGS)

section .multiboot
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

section .bss
align 16
stack_bottom:
    resb 16384              ; 16 KiB stack
stack_top:

section .text
global _start
extern kernel_main

idle:
    hlt
    jmp idle

_start:
    mov esp, stack_top
    call kernel_main
    jmp idle