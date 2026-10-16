/* keyboard.c */
#include "io.h"

#define KBD_STATUS_PORT 0x64
#define KBD_DATA_PORT   0x60

/* Tell the compiler this function exists in terminal.c */
extern void terminal_putchar(char c);
extern void terminal_lock_input(void);


/* AZERTY (French) Scan Code Set 1 Lookup Table */
static const char kbd_azerty[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', ')', '=', '\b',
  '\t', 'a', 'z', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '^', '$', '\n',
    0, /* 29 - Left Control */
  'q', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 'm', 'u', '*',
    0, /* 42 - Left Shift */
   '<', 'w', 'x', 'c', 'v', 'b', 'n', ',', ';', ':', '!',
    0, /* 54 - Right Shift */
  '*',
    0, /* 56 - Alt */
  ' ', /* 57 - Space bar */
    0  /* 58 - Caps lock */
};

void print_scancode_hex(uint8_t val) {
    const char hex_chars[] = "0123456789ABCDEF";
    terminal_putchar('0');
    terminal_putchar('x');
    terminal_putchar(hex_chars[val >> 4]);     /* Get high 4 bits */
    terminal_putchar(hex_chars[val & 0x0F]);   /* Get low 4 bits */
    terminal_putchar(' ');
}

uint8_t read_scancode(void) {
    /* Wait until bit 0 of the Status Register is 1 (Data ready to read) */
    while ((inb(KBD_STATUS_PORT) & 0x01) == 0) {
        ; 
    }
    return inb(KBD_DATA_PORT);
}

extern void terminal_switch(int new_screen);

void keyboard_loop(void) {
    while (1) {
        uint8_t scancode = read_scancode();

        /* Ignore key releases (when bit 7 is 1) */
        if (scancode & 0x80) {
            continue;
        }

        /* Virtual Terminal Switching (F1 - F4) */
        if (scancode == 0x3B) { terminal_switch(0); continue; }
        if (scancode == 0x3C) { terminal_switch(1); continue; }
        if (scancode == 0x3D) { terminal_switch(2); continue; }
        if (scancode == 0x3E) { terminal_switch(3); continue; }

        /* Translate the scancode to an ASCII character using AZERTY */
        char c = kbd_azerty[scancode];
        
        if (c != 0) {
	    terminal_putchar(c);
            if (c == '\n') {
                terminal_lock_input(); /* Lock the new line */
            }
        }
    }
}

