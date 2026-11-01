#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include "screen.h"
#include "io.h"

volatile uint16_t *const vga = (volatile uint16_t *)0xB8000;
int cursor_row = 0;
int cursor_col = 0;

uint8_t terminal_color = VGA_COLOR_WHITE_BLUE;

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

    outb(CURSOR_CMD, 0x0F);              /* select cursor low byte  */
    outb(CURSOR_DATA, pos & 0xFF);
    outb(CURSOR_CMD, 0x0E);              /* select cursor high byte */
    outb(CURSOR_DATA, pos >> 8);
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
        (uint16_t) c | (uint16_t) terminal_color << 8;

    if (++cursor_col >= VGA_WIDTH)
        vga_newline();
    vga_update_cursor();
}

void vga_print(const char *s)
{
    if(!s)
        return;
    while(*s)
        vga_putchar(*(s++));
}

void vga_clear(void)
{
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
        vga[i] = ((uint16_t)VGA_COLOR_WHITE_BLUE << 8) | ' ';
    cursor_row = cursor_col = 0;
    vga_update_cursor();
}

void vga_backspace(void)
{
    if (cursor_col > 0) {
        cursor_col--;
    } else if (cursor_row > 0) {
        cursor_row--;
        cursor_col = VGA_WIDTH - 1;
    } else {
        return;
    }
    vga[cursor_row * VGA_WIDTH + cursor_col] =
        ((uint16_t)VGA_COLOR_WHITE_BLUE << 8) | ' ';
    vga_update_cursor();
}


void terminal_change_color(void) {
    /* Extract the current foreground color and increment it */
    uint8_t fg = (terminal_color + 1) & 0x0F;
    
    /* If the foreground becomes 1 (blue text on blue background), skip to 2 (green) */
    if (fg == 1)
        fg = 2;
    /* Combine the blue background (0x10) with the new foreground color */
    terminal_color = 0x10 | fg;
}