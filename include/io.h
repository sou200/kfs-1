#ifndef IO_H
#define IO_H

#include <stdint.h>


#define CURSOR_CMD   0x3D4
#define CURSOR_DATA  0x3D5

#define KEYBOARD_SCANCODE 0x60

#define PIC1_CMD   0x20
#define PIC1_DATA  0x21
#define PIC2_CMD   0xA0
#define PIC2_DATA  0xA1

#define ICW1_INIT  0x11
#define ICW4_8086  0x01

void outb(uint16_t port, uint8_t val);
uint8_t inb(uint16_t port);
void io_wait(void);
void eoi();
void pic_remap(uint8_t offset1, uint8_t offset2);

#endif