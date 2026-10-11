extern void printf(char *str);

void kernel_main() {
    asm volatile ("cli");
    pic_remap(0x20, 0x28);
    idt_init();
    asm volatile ("sti");
    vga_print("hello\nworld");
}