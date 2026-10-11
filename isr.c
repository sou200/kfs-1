#include <stdint.h>

void timer_handler()
{
    return;
}

void keyboard_handler()
{
    uint8_t scancode;
    __asm__ volatile ("inb %1, %0" : "=a"(scancode) : "Nd"((uint16_t)0x60));
    vga_putchar(scancode);
}

void IRQ_handler(uint32_t int_code)
{
    if (int_code == 32)
        timer_handler();
    if (int_code == 33)
        keyboard_handler();
    __asm__ volatile ("outb %0, %1" : : "a"((uint8_t)0x20), "Nd"((uint16_t)0x20));  /* EOI */
}