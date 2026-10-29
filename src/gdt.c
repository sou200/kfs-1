#include <stdint.h>
#include "screen.h"

// Defines one GDT entry (8 bytes)
struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
} __attribute__((packed));

// Defines the pointer to the GDT (6 bytes)
struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));



void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    // Point exactly to address 0x800
    struct gdt_entry *gdt = (struct gdt_entry *)0x00000800;
    
    gdt[num].base_low    = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high   = (base >> 24) & 0xFF;

    gdt[num].limit_low   = (limit & 0xFFFF);
    gdt[num].granularity = (limit >> 16) & 0x0F;
    
    gdt[num].granularity |= gran & 0xF0;
    gdt[num].access      = access;
}

extern void gdt_flush(uint32_t pointer);

void init_gdt() {
    struct gdt_ptr gp;
    gp.limit = (sizeof(struct gdt_entry) * 7) - 1;
    gp.base  = 0x00000800; // Pointer to our hardcoded location

    gdt_set_gate(0, 0, 0, 0, 0);                // Null segment
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // Kernel Code 
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // Kernel Data 
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0x96, 0xCF); // Kernel Stack
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xFA, 0xCF); // User Code 
    gdt_set_gate(5, 0, 0xFFFFFFFF, 0xF2, 0xCF); // User Data 
    gdt_set_gate(6, 0, 0xFFFFFFFF, 0xF6, 0xCF); // User Stack

    // Call the assembly function to flush the CPU registers
    gdt_flush((uint32_t)&gp);
}


void print_stack() {
    uint32_t *esp, *ebp;

    // Grab current registers
    __asm__ volatile("mov %%esp, %0" : "=r"(esp));
    __asm__ volatile("mov %%ebp, %0" : "=r"(ebp));

    // Ensure you have a puts function ready to format hex!
    puts("--- Kernel Stack Dump ---\n");
    puts("ESP: 0x%x | EBP: 0x%x\n", (uint32_t)esp, (uint32_t)ebp);
    
    // The stack grows downward, so ESP is lower than EBP in memory.
    // We print from the top of the frame (ESP) down to the base (EBP).
    uint32_t *current = esp;
    while (current <= ebp) {
        puts("[0x%x]: 0x%x\n", (uint32_t)current, *current);
        current++; // Move up in memory (down the stack)
    }
    puts("-------------------------\n");
}


