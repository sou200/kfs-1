#include "screen.h"
#include "io.h"
#include "keyboard.h"

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

static int caps_lock  = 0;

char scancode_to_char(uint8_t sc)
{
    int released = sc & 0x80;
    if (released)
        return 0;
        
    if (sc == 0x0E)
    {
        return '\b'; // Pass backspace to the shell!
    }

    if (sc == 0x3A) {
        caps_lock = !caps_lock;
        return 0;
    }

    return caps_lock ? shifted[sc] : normal[sc];
}

// Make sure to declare handle_keypress so this file knows it exists
extern void handle_keypress(char ascii);

void keyboard_handler()
{
    uint8_t scancode;
    uint8_t character;
    asm volatile ("inb %1, %0" : "=a"(scancode) : "Nd"(KEYBOARD_SCANCODE));

    // FIX 1: Ignore key releases (break codes)
    if (scancode & 0x80) {
        return; // Do nothing on key release
    }

    if (scancode == F1_SCANCODE)
        terminal_switch(0);
    else if (scancode == F2_SCANCODE)
        terminal_switch(1);
    else if (scancode == F3_SCANCODE)
        terminal_switch(2);
    else if (scancode == F4_SCANCODE)
        terminal_switch(3);
    else if (scancode == F5_SCANCODE)
        terminal_change_color();
    else
    {
        character = scancode_to_char(scancode);
        if (character) {
            // FIX 2: Send the character to the shell!
            // handle_keypress will take care of printing it AND buffering it.
            handle_keypress(character); 
        }
    }
}