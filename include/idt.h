#ifndef IDT_H
#define IDT_H

#define TIMER_INTERRUPT 32
#define KEYBOARD_INTERRUPT 33

void idt_init(void);

#endif