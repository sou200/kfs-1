; boot.s (NASM syntax)
MBALIGN  equ  1 << 0            ; Align loaded modules on page boundaries
MEMINFO  equ  1 << 1            ; Provide memory map
FLAGS    equ  MBALIGN | MEMINFO ; Multiboot flag field
MAGIC    equ  0x1BADB002        ; Magic number lets GRUB find the header
CHECKSUM equ -(MAGIC + FLAGS)   ; Checksum required (MAGIC + FLAGS + CHECKSUM = 0)

section .multiboot
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

section .bss
align 16
stack_bottom:
    resb 16384 ; Reserve 16 KB for the kernel stack
stack_top:

section .text
global _start
extern kmain   ; Defined in kernel.c

_start:
    mov esp, stack_top ; Set up stack pointer
    call kmain         ; Jump into C code
    cli                ; Disable hardware interrupts
.hang:
    hlt                ; Halt CPU
    jmp .hang          ; Loop if NMI wakes CPU
