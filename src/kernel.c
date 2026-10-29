#include "screen.h"
#include "io.h"
#include "idt.h"
#include "gdt.h"

extern void idt_init(void);

void kernel_main() {
    // 1. Disable interrupts while setting up core hardware
    asm volatile ("cli");
    
    // 2. Initialize core hardware
    init_terminals();      // Screen
    pic_remap(0x20, 0x28); // PIC 
    init_gdt();            // GDT
    idt_init();            // IDT (make sure your keyboard ISR is registered here!)
    
    // 3. Re-enable interrupts so the keyboard can talk to the CPU
    asm volatile ("sti");
    
    // 4. Print welcome message and initial shell prompt
    puts("Welcome to Kernel From Scratch (kfs-2)!\n");
    puts("kfs-2> ");
    
    // 5. Keep the kernel alive indefinitely
    // The CPU will sleep (hlt) until an interrupt (like a keystroke) wakes it up.
    while (1) {
        asm volatile ("hlt");
        
    }
}