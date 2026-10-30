#include <stdint.h>
#include "screen.h"

struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
} __attribute__((packed));

struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));



void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
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

    gdt_flush((uint32_t)&gp);
}

extern uint8_t stack_bottom[];
extern uint8_t stack_top[];

void print_stack(uint32_t max_words) {
    uint32_t *esp, *ebp;
    uint32_t *stack_start = (uint32_t *)stack_bottom;
    uint32_t *stack_end = (uint32_t *)stack_top;
    uint32_t *current;
    uint32_t *upper_bound;
    uint32_t words = 0;
    uint32_t available_words;

    if (max_words == 0)
        max_words = 12;
    else if (max_words > 256)
        max_words = 256;

    __asm__ volatile("mov %%esp, %0" : "=r"(esp));
    __asm__ volatile("mov %%ebp, %0" : "=r"(ebp));

    printk("--- Kernel Stack Dump ---\n");
    printk("ESP: 0x%x | EBP: 0x%x\n", (uint32_t)esp, (uint32_t)ebp);

    if (esp < stack_start || esp >= stack_end || ebp < esp) {
        printk("Stack frame is outside the bootstrap stack.\n");
        printk("-------------------------\n");
        return;
    }

    current = esp;
    if (ebp > esp && ebp < stack_end) {
        upper_bound = ebp;
    } else {
        upper_bound = stack_end - 1;
        printk("EBP frame window unavailable; using stack top bound.\n");
    }

    available_words = (uint32_t)(upper_bound - current) + 1;
    if (available_words > max_words)
        available_words = max_words;

    while (words < available_words) {
        printk("[0x%x]: 0x%x\n", (uint32_t)current, *current);
        current++;
        words++;
    }
    if (current <= upper_bound)
        printk("... %d more stack words omitted (use 'stack N').\n",
               (int32_t)(upper_bound - current + 1));
    printk("-------------------------\n");
}


