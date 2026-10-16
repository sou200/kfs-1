/* kmain.c */
extern void terminal_initialize(void);
extern void terminal_putchar(char c);
extern void enable_cursor(uint8_t cursor_start, uint8_t cursor_end);
extern void keyboard_loop(void);

/* Helper function to print strings */
void terminal_writestring(const char* data) {
    for (int i = 0; data[i] != '\0'; i++) {
        terminal_putchar(data[i]);
    }
}

extern void printk(const char* format, ...);

void kmain(void) {
    terminal_initialize();
    enable_cursor(14, 15);

    printk("42\n");
    printk("Welcome to KFS-1, %s!\n", "student");
    printk("Current hex test: %x\n", 0x1337);
    printk("Press F1-F4 to switch virtual terminals.\n> ");

    keyboard_loop();
}

