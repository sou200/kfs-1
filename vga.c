#include <stdint.h>

#define VGA_WIDTH  80
#define VGA_HEIGHT 25
#define VGA_COLOR  0x0F   /* white on black */

static volatile uint16_t *const vga = (volatile uint16_t *)0xB8000;
static int cursor_row = 0;
static int cursor_col = 0;

static void vga_scroll(void)
{
    for (int i = 0; i < (VGA_HEIGHT - 1) * VGA_WIDTH; i++)
        vga[i] = vga[i + VGA_WIDTH];

    for (int i = (VGA_HEIGHT - 1) * VGA_WIDTH; i < VGA_HEIGHT * VGA_WIDTH; i++)
        vga[i] = ((uint16_t)VGA_COLOR << 8) | ' ';

    cursor_row = VGA_HEIGHT - 1;
}

static void vga_newline(void)
{
    cursor_col = 0;
    if (++cursor_row >= VGA_HEIGHT)
        vga_scroll();
}

void vga_putchar(char c)
{
    if (c == '\n') {
        vga_newline();
        return;
    }
    if (c == '\r') {
        cursor_col = 0;
        return;
    }

    vga[cursor_row * VGA_WIDTH + cursor_col] =
        ((uint16_t)VGA_COLOR << 8) | (uint8_t)c;

    if (++cursor_col >= VGA_WIDTH)
        vga_newline();
}

void vga_print(const char *s)
{
    for (; *s; s++)
        vga_putchar(*s);
}

void vga_clear(void)
{
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
        vga[i] = ((uint16_t)VGA_COLOR << 8) | ' ';
    cursor_row = cursor_col = 0;
}