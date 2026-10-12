#ifndef SCREEN_H
#define SCREEN_H

#include <stdint.h>

#define VGA_WIDTH  80
#define VGA_HEIGHT 25
#define VGA_COLOR_WHITE_BLACK  0x0F   /* white on black */
#define VGA_COLOR_WHITE_BLUE  0x1F   /* white on black */

void vga_putchar(char c);
void vga_print(const char *s);
void vga_clear(void);

#endif