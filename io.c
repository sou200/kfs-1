#include <stdint.h>

#define PIC1_CMD   0x20
#define PIC1_DATA  0x21
#define PIC2_CMD   0xA0
#define PIC2_DATA  0xA1

#define ICW1_INIT  0x10
#define ICW1_ICW4  0x01
#define ICW4_8086  0x01

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Short delay for old hardware: write to an unused port
static inline void io_wait(void) {
    outb(0x80, 0);
}

void pic_remap(uint8_t offset1, uint8_t offset2) {
    // Save current masks
    uint8_t mask1 = inb(PIC1_DATA);
    uint8_t mask2 = inb(PIC2_DATA);

    // ICW1: begin initialization
    outb(PIC1_CMD, ICW1_INIT | ICW1_ICW4); io_wait();
    outb(PIC2_CMD, ICW1_INIT | ICW1_ICW4); io_wait();

    // ICW2: vector offsets
    outb(PIC1_DATA, offset1); io_wait();   // master: 0x20
    outb(PIC2_DATA, offset2); io_wait();   // slave:  0x28

    // ICW3: wiring
    outb(PIC1_DATA, 0x04); io_wait();      // master: slave is on IRQ2 (bit 2)
    outb(PIC2_DATA, 0x02); io_wait();      // slave: its cascade identity is 2

    // ICW4: 8086 mode
    outb(PIC1_DATA, ICW4_8086); io_wait();
    outb(PIC2_DATA, ICW4_8086); io_wait();

    // Restore masks
    outb(PIC1_DATA, mask1);
    outb(PIC2_DATA, mask2);
}