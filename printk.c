/* printk.c */
extern void terminal_putchar(char c);
extern void terminal_writestring(const char* data);

/* Use GCC built-ins for variadic arguments */
typedef __builtin_va_list va_list;
#define va_start(v, l) __builtin_va_start(v, l)
#define va_arg(v, l)   __builtin_va_arg(v, l)
#define va_end(v)      __builtin_va_end(v)

/* Helper to print base-10 integers */
static void print_itoa(int num) {
    if (num == 0) {
        terminal_putchar('0');
        return;
    }
    if (num < 0) {
        terminal_putchar('-');
        num = -num;
    }
    char buffer[12];
    int i = 0;
    while (num > 0) {
        buffer[i++] = (num % 10) + '0';
        num /= 10;
    }
    while (--i >= 0) {
        terminal_putchar(buffer[i]);
    }
}

/* Helper to print base-16 hexadecimals */
static void print_hex(unsigned int num) {
    const char hex_chars[] = "0123456789abcdef";
    char buffer[8];
    int i = 0;
    if (num == 0) {
        terminal_putchar('0');
        return;
    }
    while (num > 0) {
        buffer[i++] = hex_chars[num % 16];
        num /= 16;
    }
    while (--i >= 0) {
        terminal_putchar(buffer[i]);
    }
}

/* The main formatting function */
void printk(const char* format, ...) {
    va_list args;
    va_start(args, format);

    for (int i = 0; format[i] != '\0'; i++) {
        if (format[i] == '%' && format[i + 1] != '\0') {
            i++; /* Skip the '%' */
            switch (format[i]) {
                case 'c':
                    terminal_putchar((char)va_arg(args, int));
                    break;
                case 's':
                    terminal_writestring(va_arg(args, char*));
                    break;
                case 'd':
                    print_itoa(va_arg(args, int));
                    break;
                case 'x':
                    terminal_writestring("0x");
                    print_hex(va_arg(args, unsigned int));
                    break;
                case '%':
                    terminal_putchar('%');
                    break;
                default:
                    terminal_putchar('%');
                    terminal_putchar(format[i]);
                    break;
            }
        } else {
            terminal_putchar(format[i]);
        }
    }
    va_end(args);
}
