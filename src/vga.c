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

static void print_number(uint32_t num, int base, int is_signed) {
    char buffer[32];
    const char *digits = "0123456789abcdef";
    int i = 0;
    int is_negative = 0;

    if (is_signed && base == 10 && (int32_t)num < 0) {
        is_negative = 1;
        num = 0u - num;
    }

    if (num == 0) {
        buffer[i++] = '0';
    }

    while (num > 0) {
        buffer[i++] = digits[num % base];
        num /= base;
    }

    if (is_negative) {
        buffer[i++] = '-';
    }

    // Reverse the string and print it
    while (--i >= 0) {
        vga_putchar(buffer[i]);
    }
}

void vprintk(const char *format, va_list args) {
    for (size_t i = 0; format[i] != '\0'; i++) {
        if (format[i] == '%' && format[i + 1] != '\0') {
            i++; // Skip the '%'
            switch (format[i]) {
                case 'c': {
                    // char is promoted to int in variadic arguments
                    char c = (char)va_arg(args, int);
                    vga_putchar(c);
                    break;
                }
                case 's': {
                    const char *s = va_arg(args, const char *);
                    vga_print(s ? s : "(null)");
                    break;
                }
                case 'd': {
                    int num = va_arg(args, int);
                    print_number(num, 10, 1);
                    break;
                }
                case 'x': {
                    uint32_t num = va_arg(args, uint32_t);
                    print_number(num, 16, 0);
                    break;
                }
                case '%': {
                    vga_putchar('%');
                    break;
                }
                default: {
                    // If it's an unsupported format, just print it as-is
                    vga_putchar('%');
                    vga_putchar(format[i]);
                    break;
                }
            }
        } else {
            vga_putchar(format[i]);
        }
    }

}

void printk(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vprintk(format, args);
    va_end(args);
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

// void terminal_switch(int new_screen) {
//     if (new_screen < 0 || new_screen >= NUM_SCREENS || new_screen == active_screen) {
//         return;
//     }

//     /* Save current VGA memory and cursor to the active screen's backup */
//     for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
//         screen_buffers[active_screen][i] = vga[i];
//     }
//     cursor_x[active_screen] = cursor_col;
//     cursor_y[active_screen] = cursor_row;

//     /* Update the active screen index */
//     active_screen = new_screen;

//     /* Load the new screen's backup into physical VGA memory */
//     for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
//         vga[i] = screen_buffers[active_screen][i];
//     }
//     cursor_col = cursor_x[active_screen];
//     cursor_row = cursor_y[active_screen];

//     /* Move the hardware cursor to match the new screen */
//     vga_update_cursor();
// }

// void init_terminals() {
//     active_screen = 0;
//     cursor_row = 0;
//     cursor_col = 0;
//     for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
//         vga[i] = screen_buffers[0][i];
//     vga_update_cursor();
// }