#include "screen.h"
#include "io.h"
#include "idt.h"
#include "gdt.h"

extern void idt_init(void);

void kernel_main() {
    // 1. Disable interrupts while setting up core hardware
    asm volatile ("cli");
    
    // 2. Initialize core hardware
    pic_remap(0x20, 0x28); // PIC 
    init_gdt();            // GDT
    idt_init();             // IDT (make sure your keyboard ISR is registered here!)
    vga_clear();          
    
    // 3. Re-enable interrupts so the keyboard can talk to the CPU
    asm volatile ("sti");
    
    // 4. Print welcome message and initial shell prompt
    printk("Welcome to Kernel From Scratch (kfs-2)!\n");
    printk("kfs-2> ");
}