#include "screen.h"
#include "helper.h"


static const char hex_digits[] = "0123456789abcdef";

static void hex_byte(char *out, uint8_t v)
{
    out[0] = hex_digits[v >> 4];
    out[1] = hex_digits[v & 0xF];
}

static void hex_u32(char *out, uint32_t v)
{
    for (int i = 7; i >= 0; i--) {
        out[i] = hex_digits[v & 0xF];
        v >>= 4;
    }
}

void dump_stack(uint32_t start, uint32_t end)
{

    if (start > end) {
        uint32_t t = start; start = end; end = t;
    }

    for (uint32_t addr = start; addr < end; addr += 16) {
        char line[80];
        char *p = line;

        uint32_t n = end - addr;      /* bytes left */
        if (n > 16) n = 16;

        hex_u32(p, addr);
        p += 8;
        *p++ = ':';
        *p++ = ' ';

        for (uint32_t i = 0; i < 16; i++) {
            if (i < n) {
                hex_byte(p, *(volatile uint8_t *)(addr + i));
            } else {
                p[0] = ' ';           /* pad only on the last line */
                p[1] = ' ';
            }
            p += 2;

            if (i & 1)
                *p++ = ' ';
        }

        *p++ = ' ';

        for (uint32_t i = 0; i < n; i++) {
            uint8_t c = *(volatile uint8_t *)(addr + i);
            *p++ = (c >= 0x20 && c <= 0x7E) ? (char)c : '.';
        }
        *p = '\0';
        printk("%s\n", line);
    }
}