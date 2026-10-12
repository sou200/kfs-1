#include "io.h"
#include "idt.h"

void keyboard_handler();

void IRQ_handler(uint32_t int_code)
{
    if (int_code == KEYBOARD_INTERRUPT)
        keyboard_handler();
    eoi();
}