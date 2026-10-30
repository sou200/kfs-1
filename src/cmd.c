#define BUFFER_SIZE 256
#include <stdint.h>
#include "screen.h"
#include "cmd.h"
#include "gdt.h"
#include "io.h"

extern void dump_stack(uint32_t esp, uint32_t ebp);

static char cmd_buffer[BUFFER_SIZE];
static int cmd_index = 0;

#define HELP_MSG "Available Commands\n\
\n\
  help        Show this help message\n\
  halt        Halt the system\n\
  reboot       Reboot the system\n\
  stack       Display the current stack contents\n\
\n\
Type 'help' to list available commands.\n"

// A simple string comparison helper
int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++; s2++;
    }
    return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

void handle_keypress(char ascii) {
    if (ascii == '\n' || ascii == '\r') {
        vga_putchar('\n');
        cmd_buffer[cmd_index] = '\0';
        execute_command(cmd_buffer);
        
        cmd_index = 0;
        puts("kfs-2> ");
    } 
    else if (ascii == '\b') {
        // Protects the prompt! Only delete if we typed something.
        if (cmd_index > 0) {
            cmd_index--;     // 1. Remove from shell memory
            vga_backspace(); // 2. Remove from the screen
        }
    } 
    else if (cmd_index < BUFFER_SIZE - 1) {
        cmd_buffer[cmd_index++] = ascii;
        vga_putchar(ascii);
    }
}

void execute_command(char *cmd) {
    uint32_t esp, ebp;
    if (cmd[0] == '\0') {
        return; // Empty command, do nothing
    }
    
    if (strcmp(cmd, "stack") == 0) {
        __asm__ volatile ("mov %%esp, %0" : "=r"(esp));
        __asm__ volatile ("mov %%ebp, %0" : "=r"(ebp));
        dump_stack(esp, ebp);
    } 
    else if (strcmp(cmd, "halt") == 0) {
        puts("Halting system...\n");
        __asm__ volatile("cli; hlt"); // Disable interrupts and halt CPU
    }  else if (strcmp(cmd, "help") == 0) {
        puts(HELP_MSG);
    } 
    else if (strcmp(cmd, "reboot") == 0) {
        puts("Rebooting...\n");
        uint8_t good = 0x02;
        while (good & 0x02) {
            good = inb(0x64); // Wait for keyboard controller to be ready
        }
        outb(0x64, 0xFE); // Pulse the CPU reset line via PS/2 controller
    } 
    else {
        puts("Unknown command: ");
        puts(cmd);
        puts("\nRaw bytes: ");
        
        // Print the exact hex values hiding inside the string
        for (int i = 0; cmd[i] != '\0'; i++) {
            // Assuming your printk can print hex. If not, use your screen 
            // printing function to show the raw integer values.
            puts("[%x] ", (uint8_t)cmd[i]); 
        }
        puts("\n");
        puts("Type 'help' to list available commands.\n");
    }
}