#include "screen.h"
#include "io.h"

/* Index = scancode. Unlisted entries are 0 (no character). */
static const char normal[128] = {
    0, 27,                                                  /* 0x00 - 0x01 */
    '1','2','3','4','5','6','7','8','9','0','-','=',        /* 0x02 - 0x0D */
    '\b','\t',                                              /* 0x0E - 0x0F */
    'q','w','e','r','t','y','u','i','o','p','[',']',        /* 0x10 - 0x1B */
    '\n',                                                   /* 0x1C enter  */
    0,                                                      /* 0x1D ctrl   */
    'a','s','d','f','g','h','j','k','l',';','\'','`',       /* 0x1E - 0x29 */
    0,                                                      /* 0x2A l-shift*/
    '\\',                                                   /* 0x2B        */
    'z','x','c','v','b','n','m',',','.','/',                /* 0x2C - 0x35 */
    0,                                                      /* 0x36 r-shift*/
    '*',                                                    /* 0x37 keypad */
    0,                                                      /* 0x38 alt    */
    ' ',                                                    /* 0x39 space  */
};

static const char shifted[128] = {
    0, 27,
    '!','@','#','$','%','^','&','*','(',')','_','+',
    '\b','\t',
    'Q','W','E','R','T','Y','U','I','O','P','{','}',
    '\n',
    0,
    'A','S','D','F','G','H','J','K','L',':','"','~',
    0,
    '|',
    'Z','X','C','V','B','N','M','<','>','?',
    0,
    '*',
    0,
    ' ',
};

static int shift_down = 0;
static int caps_lock  = 0;

/* Feed it the raw byte from port 0x60.
 * Returns the ASCII character, or 0 if the byte isn't a printable key press. */
char scancode_to_char(uint8_t sc)
{
    int released = sc & 0x80;       /* bit 7 set = key release */
    if (released)
        return 0;
    if (sc == 0x0E)
    {
        vga_backspace();
        return 0;
    }

    if (sc == 0x3A) {               /* caps lock toggles on press */
        caps_lock = !caps_lock;
        return 0;
    }

    return caps_lock ? shifted[sc] : normal[sc];
}

void keyboard_handler()
{
    uint8_t scancode;
    uint8_t character;
    asm volatile ("inb %1, %0" : "=a"(scancode) : "Nd"(KEYBOARD_SCANCODE));
    character = scancode_to_char(scancode);
    if (character)
        vga_putchar(character);
}