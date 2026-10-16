/* terminal.c */
#include "io.h"

#define VGA_ADDRESS 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define NUM_SCREENS 4

/* 1. Global Terminal State Variables */
static uint16_t* terminal_buffer;
static int terminal_row;
static int terminal_column;
static uint8_t terminal_color;
static int protect_x = 0;
static int protect_y = 0;

void terminal_lock_input(void) {
    protect_x = terminal_column;
    protect_y = terminal_row;
}

/* 2. Virtual Screen State Variables */
static uint16_t screen_buffers[NUM_SCREENS][VGA_WIDTH * VGA_HEIGHT];
static int cursor_x[NUM_SCREENS];
static int cursor_y[NUM_SCREENS];
static int active_screen = 0;

/* 3. Forward Declarations (so the compiler knows these exist) */
extern void update_cursor(int x, int y);
void terminal_switch(int new_screen);

/* 4. Utility: Combine character and color */
static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t) uc | (uint16_t) color << 8;
}

void terminal_initialize(void) {
    terminal_color = 0x0F;
    terminal_buffer = (uint16_t*) VGA_ADDRESS;
    active_screen = 0;

    /* Clear all hidden screen buffers */
    for (int s = 0; s < NUM_SCREENS; s++) {
        cursor_x[s] = 0;
        cursor_y[s] = 0;
        for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
            screen_buffers[s][i] = vga_entry(' ', terminal_color);
        }
    }

    /* FIX 1: Explicitly clear the physical VGA buffer on boot */
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        terminal_buffer[i] = vga_entry(' ', terminal_color);
    }
    
    terminal_row = 0;
    terminal_column = 0;
    protect_x = 0;
    protect_y = 0;
}

/* 6. Virtual Screen Switching */
void terminal_switch(int new_screen) {
    if (new_screen < 0 || new_screen >= NUM_SCREENS || new_screen == active_screen) {
        return;
    }

    /* Save current VGA memory and cursor to the active screen's backup */
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        screen_buffers[active_screen][i] = terminal_buffer[i];
    }
    cursor_x[active_screen] = terminal_column;
    cursor_y[active_screen] = terminal_row;

    /* Update the active screen index */
    active_screen = new_screen;

    /* Load the new screen's backup into physical VGA memory */
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        terminal_buffer[i] = screen_buffers[active_screen][i];
    }
    terminal_column = cursor_x[active_screen];
    terminal_row = cursor_y[active_screen];

    /* Move the hardware cursor to match the new screen */
    update_cursor(terminal_column, terminal_row);
}

/* 7. Scrolling */
static void terminal_scroll(void) {
    /* Move rows 1-24 up by one */
    for (int y = 1; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            int to_index = (y - 1) * VGA_WIDTH + x;
            int from_index = y * VGA_WIDTH + x;
            terminal_buffer[to_index] = terminal_buffer[from_index];
        }
    }

    /* Clear the last row */
    for (int x = 0; x < VGA_WIDTH; x++) {
        int index = (VGA_HEIGHT - 1) * VGA_WIDTH + x;
        terminal_buffer[index] = vga_entry(' ', terminal_color);
    }
    terminal_row = VGA_HEIGHT - 1;
}

/* 8. Output a single character */
void terminal_putchar(char c) {
    if (c == '\n') {
        terminal_column = 0;
        terminal_row++;
    } else {
        /* Standard character */
        int index = terminal_row * VGA_WIDTH + terminal_column;
        terminal_buffer[index] = vga_entry(c, terminal_color);
        terminal_column++;
    }

    /* Wrap to next line if we hit the right edge */
    if (terminal_column >= VGA_WIDTH) {
        terminal_column = 0;
        terminal_row++;
    }

    /* Scroll if we hit the bottom */
    if (terminal_row >= VGA_HEIGHT) {
        terminal_scroll();
    }

    update_cursor(terminal_column, terminal_row);
}


/* 9. Output a string */
void terminal_writestring(const char* data) {
    for (int i = 0; data[i] != '\0'; i++) {
        terminal_putchar(data[i]);
    }
}

