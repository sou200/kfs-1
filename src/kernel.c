#include "screen.h"
#include "io.h"
#include "idt.h"

extern void idt_init(void);

void kernel_main() {
    asm volatile ("cli");
    pic_remap(0x20, 0x28);
    idt_init();
    asm volatile ("sti");
    vga_clear();
    vga_print("hello\nworld");
}