#include <stdint.h>
#include "screen.h"
#include "io.h"

static volatile uint16_t *const vga = (volatile uint16_t *)0xB8000;
static int cursor_row = 0;
static int cursor_col = 0;

static void vga_scroll(void)
{
    for (int i = 0; i < (VGA_HEIGHT - 1) * VGA_WIDTH; i++)
        vga[i] = vga[i + VGA_WIDTH];

    for (int i = (VGA_HEIGHT - 1) * VGA_WIDTH; i < VGA_HEIGHT * VGA_WIDTH; i++)
        vga[i] = ((uint16_t)VGA_COLOR_WHITE_BLUE << 8) | ' ';

    cursor_row = VGA_HEIGHT - 1;
}


void vga_update_cursor(void)
{
    uint16_t pos = cursor_row * VGA_WIDTH + cursor_col;

    outb(0x3D4, 0x0F);              /* select cursor low byte  */
    outb(0x3D5, pos & 0xFF);
    outb(0x3D4, 0x0E);              /* select cursor high byte */
    outb(0x3D5, pos >> 8);
}

static void vga_newline(void)
{
    cursor_col = 0;
    if (++cursor_row >= VGA_HEIGHT)
        vga_scroll();
    vga_update_cursor();
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
        ((uint16_t)VGA_COLOR_WHITE_BLUE << 8) | (uint8_t)c;

    if (++cursor_col >= VGA_WIDTH)
        vga_newline();
    vga_update_cursor();
}

void vga_print(const char *s)
{
    for (; *s; s++)
        vga_putchar(*s);
}

void vga_clear(void)
{
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
        vga[i] = ((uint16_t)VGA_COLOR_WHITE_BLUE << 8) | ' ';
    cursor_row = cursor_col = 0;
    vga_update_cursor();
}