/* idt.c - simplest possible IDT (works with isr.asm) */
#include <stdint.h>

struct idt_entry {
    uint16_t base_lo;
    uint16_t selector;
    uint8_t  zero;
    uint8_t  flags;
    uint16_t base_hi;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

extern void IRQ0(void);     /* stubs from isr.asm */
extern void IRQ1(void);

static struct idt_entry idt[256];
static struct idt_ptr   idtp;


static void idt_set_gate(int n, void (*handler)(void), uint16_t sel) {
    uint32_t addr = (uint32_t)handler;
    idt[n].base_lo  = addr & 0xFFFF;
    idt[n].base_hi  = addr >> 16;
    idt[n].selector = sel;
    idt[n].zero     = 0;
    idt[n].flags    = 0x8E;     /* present, ring 0, interrupt gate */
}

void idt_init(void) {
    uint16_t cs;
    asm volatile ("mov %%cs, %0" : "=r"(cs));   /* GRUB's code selector */

    idt_set_gate(32, IRQ0, cs);
    idt_set_gate(33, IRQ1, cs);

    idtp.limit = sizeof(idt) - 1;
    idtp.base  = (uint32_t)&idt;
    asm volatile ("lidt %0" : : "m"(idtp));
}