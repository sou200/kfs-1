global gdt_flush

gdt_flush:
    mov eax, [esp+4]  ; Get the pointer passed from C
    lgdt [eax]        ; Load the GDT into the CPU

    ; Update data segment registers (offset 0x10 is Kernel Data)
    mov ax, 0x10      
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Update stack segment (offset 0x18 is Kernel Stack)
    mov ax, 0x18      
    mov ss, ax

    ; Far jump to update the Code Segment (offset 0x08 is Kernel Code)
    jmp 0x08:.flush   
.flush:
    ret