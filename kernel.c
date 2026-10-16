/* kernel.c */
#include "io.h"

/* Forward declarations */
extern void terminal_initialize(void);
extern void enable_cursor(uint8_t cursor_start, uint8_t cursor_end);
extern void keyboard_loop(void);
extern void printk(const char* format, ...);
/* Add this forward declaration at the top */
extern void terminal_lock_input(void);

void kmain(void) {
    terminal_initialize();
    enable_cursor(14, 15);

    printk("42\n");
    printk("Welcome to KFS-1, %s!\n", "student");
    printk("Current hex test: %x\n", 0x1337);
    printk("Press F1-F4 to switch virtual terminals.\n> ");

    /* Lock the boundary here so the user cannot backspace past the '>' */
    terminal_lock_input();

    keyboard_loop();
}
