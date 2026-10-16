/* kernel.c */
#include "io.h"

/* Tell the compiler these functions exist in your other files */
extern void terminal_initialize(void);
extern void terminal_putchar(char c);
extern void enable_cursor(uint8_t cursor_start, uint8_t cursor_end);
extern void keyboard_loop(void);

/* Helper function to print full strings instead of one char at a time */
void terminal_writestring(const char* data) {
    for (int i = 0; data[i] != '\0'; i++) {
        terminal_putchar(data[i]);
    }
}

void kmain(void) {
    /* 1. Setup the screen and cursor */
    terminal_initialize();
    enable_cursor(14, 15);

    /* 2. Print the required project output */
    terminal_writestring("42\n");
    terminal_writestring("> ");

    /* 3. Enter the infinite keyboard polling loop. 
       The kernel will now stay here forever, reading keys and printing them! */
    keyboard_loop();
}
