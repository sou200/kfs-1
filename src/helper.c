#include <stdint.h>
#include <stdarg.h>
#include "screen.h"

int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++; s2++;
    }
    return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

char *trim_command(char *cmd)
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

static void vprintk(const char *format, va_list args) {
    for (uint32_t i = 0; format[i] != '\0'; i++) {
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