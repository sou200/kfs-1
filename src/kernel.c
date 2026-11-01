#include "screen.h"
#include "helper.h"
#include "io.h"
#include "idt.h"
#include "gdt.h"

extern void idt_init(void);

void kernel_main() {
    asm volatile ("cli");
    init_gdt();
    pic_remap(0x20, 0x28);
    idt_init();
    vga_clear();          
    asm volatile ("sti");
    printk("Welcome to Kernel From Scratch (kfs-2)!\n");
    printk("kfs-2> ");
}