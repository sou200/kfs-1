#define BUFFER_SIZE 256
#include <stdint.h>
#include "screen.h"
#include "cmd.h"
#include "gdt.h"
#include "io.h"
#include "helper.h"

extern void dump_stack(uint32_t esp, uint32_t ebp);

static char cmd_buffer[BUFFER_SIZE];
static int cmd_index = 0;

void handle_keypress(char ascii) {
    if (ascii == '\n' || ascii == '\r') {
        vga_putchar('\n');
        cmd_buffer[cmd_index] = '\0';
        execute_command(cmd_buffer);
        
        cmd_index = 0;
        printk("kfs-2> ");
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
    cmd = trim_command(cmd);
    uint32_t esp, ebp;
    if (cmd[0] == '\0') {
        return;
    }
    
    if (strcmp(cmd, "stack") == 0) {
        __asm__ volatile ("mov %%esp, %0" : "=r"(esp));
        __asm__ volatile ("mov %%ebp, %0" : "=r"(ebp));
        dump_stack(esp, ebp);
    } 
    else if (strcmp(cmd, "halt") == 0) {
        printk("Halting system...\n");
        __asm__ volatile("cli; hlt"); // Disable interrupts and halt CPU
    }
    else if (strcmp(cmd, "reboot") == 0) {
        uint32_t retries = 10;

        printk("Rebooting...\n");
        while ((inb(0x64) & 0x02) != 0 && retries-- != 0) { 
            // inb(0x64) it must be X X X X X X 0 X to contunue
            io_wait();
        }
        if (retries == 0) {
            printk("Keyboard controller did not become ready.\n");
            return;
        }
        outb(0x64, 0xFE);
    }
    else if (strcmp(cmd, "clear") == 0) {
        vga_clear();
    }
    else if (strcmp(cmd, "echo") == 0)
    {
        return;
    }
    else if (cmd[0] == 'e' && cmd[1] == 'c' && cmd[2] == 'h' &&
               cmd[3] == 'o' &&
               (cmd[4] == ' ' || cmd[4] == '\t')) {
        printk("%s\n", cmd + 5);
    }
    else if (strcmp(cmd, "help") == 0) {
        printk("Available commands:\n");
        printk("  help   - Show this list of commands\n");
        printk("  clear  - Clear the terminal screen\n");
        printk("  echo   - Print text back to the screen\n");
        printk("  stack  - Display the current kernel stack trace\n");
        printk("  halt   - Stop CPU execution safely\n");
        printk("  reboot - Restart the system via keyboard controller\n");
        
        printk("\nKeyboard Shortcuts:\n");
        printk("  F1     - Cycle terminal font color\n");
    }
    else {
        printk("Unknown command: %s. Type 'help' for a list of commands.\n", cmd);
    }
}