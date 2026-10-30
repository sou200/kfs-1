#define BUFFER_SIZE 256
#include <stdint.h>
#include "screen.h"
#include "cmd.h"
#include "gdt.h"
#include "io.h"

static char cmd_buffer[BUFFER_SIZE];
static int cmd_index = 0;

int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++; s2++;
    }
    return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

static char *trim_command(char *cmd)
{
    char *end;

    while (*cmd == ' ' || *cmd == '\t')
        cmd++;

    end = cmd;
    while (*end != '\0')
        end++;

    while (end > cmd && (end[-1] == ' ' || end[-1] == '\t'))
        end--;
    *end = '\0';
    return cmd;
}

static uint32_t stack_word_count(const char *argument)
{
    uint32_t value = 0;

    while (*argument == ' ' || *argument == '\t')
        argument++;
    while (*argument >= '0' && *argument <= '9') {
        value = value * 10 + (uint32_t)(*argument - '0');
        argument++;
    }
    while (*argument == ' ' || *argument == '\t')
        argument++;

    return *argument == '\0' ? value : 0;
}

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
    if (cmd[0] == '\0') {
        return;
    }
    
    if (strcmp(cmd, "stack") == 0) {
        print_stack(12);
    } else if (cmd[0] == 's' && cmd[1] == 't' && cmd[2] == 'a' &&
               cmd[3] == 'c' && cmd[4] == 'k' &&
               (cmd[5] == ' ' || cmd[5] == '\t')) {
        uint32_t count = stack_word_count(cmd + 5);
        if (count == 0) {
            printk("Usage: stack [positive number of words]\n");
        } else {
            print_stack(count);
        }
    } 
    else if (strcmp(cmd, "halt") == 0) {
        printk("Halting system...\n");
        __asm__ volatile("cli; hlt");
    } 
    else if (strcmp(cmd, "reboot") == 0) {
        uint32_t retries = 1000000;

        printk("Rebooting...\n");
        while ((inb(0x64) & 0x02) != 0 && retries-- != 0) {
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
        printk("  stack  - Display the current kernel stack trace / pointer\n");
        printk("  halt   - Stop CPU execution safely\n");
        printk("  reboot - Restart the system via keyboard controller\n");
    }
    else {
        printk("Unknown command: %s. Type 'help' for a list of commands.\n", cmd);
    }
}