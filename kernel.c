#include <stdint.h>

void kernel_main() {
    volatile uint16_t *vga = (uint16_t *)0xB8000;
    const char *msg = "Hello from kernel_main!";
    for (int i = 0; msg[i]; i++)
        vga[i] = (uint16_t)msg[i] | (0x0F << 8);   // white on black
}