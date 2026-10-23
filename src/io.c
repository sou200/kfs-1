#include "io.h"

inline void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1"
        :
        : "a"(val), "Nd"(port));
}

inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0"
        : "=a"(ret)
        : "Nd"(port));
    return ret;
}

// Short delay for old hardware: write to an unused port
inline void io_wait(void) {
    outb(0x80, 0);
}

inline void eoi()
{
    outb(PIC1_CMD, 0x20);
}

void pic_remap(uint8_t offset1, uint8_t offset2) {

    outb(PIC1_CMD, ICW1_INIT);
    io_wait();
    outb(PIC2_CMD, ICW1_INIT);
    io_wait();

    // ICW2: vector offsets
    outb(PIC1_DATA, offset1);
    io_wait();
    outb(PIC2_DATA, offset2);
    io_wait();

    // ICW3: wiring
    outb(PIC1_DATA, 0x04);
    io_wait();
    outb(PIC2_DATA, 0x02);
    io_wait();

    // ICW4: 8086 mode
    outb(PIC1_DATA, ICW4_8086);
    io_wait();
    outb(PIC2_DATA, ICW4_8086);
    io_wait();

}