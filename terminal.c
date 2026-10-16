/* terminal.c */
#include "io.h" /* Your inb/outb header from earlier */

#define VGA_ADDRESS 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

static uint16_t* terminal_buffer;
static int terminal_row;
static int terminal_column;
static uint8_t terminal_color;

/* Combine character and color into a 16-bit VGA entry */
static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t) uc | (uint16_t) color << 8;
}

void terminal_initialize(void) {
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = 0x0F; /* White on Black */
    terminal_buffer = (uint16_t*) VGA_ADDRESS;

    for (int y = 0; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            const int index = y * VGA_WIDTH + x;
            terminal_buffer[index] = vga_entry(' ', terminal_color);
        }
    }
}

static void terminal_scroll(void) {
    /* Move every row up by one */
    for (int y = 1; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            int to_index = (y - 1) * VGA_WIDTH + x;
            int from_index = y * VGA_WIDTH + x;
            terminal_buffer[to_index] = terminal_buffer[from_index];
        }
    }

    /* Clear the last row (row 24) */
    for (int x = 0; x < VGA_WIDTH; x++) {
        int index = (VGA_HEIGHT - 1) * VGA_WIDTH + x;
        terminal_buffer[index] = vga_entry(' ', terminal_color);
    }

    /* Move the cursor up to the newly cleared line */
    terminal_row = VGA_HEIGHT - 1;
}

/* Bring in your cursor update function from before */
extern void update_cursor(int x, int y);

void terminal_putchar(char c) {
    if (c == '\n') {
        terminal_column = 0;
        terminal_row++;
    } else if (c == '\b') {
        /* Backspace: move back one space and overwrite with a blank */
        if (terminal_column > 0) {
            terminal_column--;
        } else if (terminal_row > 0) {
            terminal_row--;
            terminal_column = VGA_WIDTH - 1;
        }
        int index = terminal_row * VGA_WIDTH + terminal_column;
        terminal_buffer[index] = vga_entry(' ', terminal_color);
    } else {
        /* Standard character output */
        int index = terminal_row * VGA_WIDTH + terminal_column;
        terminal_buffer[index] = vga_entry(c, terminal_color);
        terminal_column++;
    }

    /* If we hit the right edge, wrap to the next line */
    if (terminal_column >= VGA_WIDTH) {
        terminal_column = 0;
        terminal_row++;
    }

    /* If we hit the bottom edge, scroll the screen up */
    if (terminal_row >= VGA_HEIGHT) {
        terminal_scroll();
    }

    /* Move the blinking hardware cursor to the new position */
    update_cursor(terminal_column, terminal_row);
}


