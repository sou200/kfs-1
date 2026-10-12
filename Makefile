CC   = gcc
NASM = nasm
LD   = ld

SRC_DIR   = src
INC_DIR   = include
BUILD_DIR = build
ISODIR    = $(BUILD_DIR)/isodir

CFLAGS    = -m32 -ffreestanding -O2 -Wall -Wextra -I$(INC_DIR)
NASMFLAGS = -f elf32
LDFLAGS   = -m elf_i386 -T linker.ld

KERNEL = $(BUILD_DIR)/kernel.bin
ISO    = $(BUILD_DIR)/mykernel.iso

ASM_SRCS = boot.asm isr_stub.asm
C_SRCS   = kernel.c vga.c io.c idt.c isr.c keyboard.c

ASM_OBJS = $(ASM_SRCS:%.asm=$(BUILD_DIR)/%.o)
C_OBJS   = $(C_SRCS:%.c=$(BUILD_DIR)/%.o)
OBJS     = $(ASM_OBJS) $(C_OBJS)

all: $(KERNEL) iso

$(KERNEL): $(OBJS) linker.ld
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.asm | $(BUILD_DIR)
	$(NASM) $(NASMFLAGS) $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

iso: $(ISO)

$(ISO): $(KERNEL) boot/grub/grub.cfg
	rm -rf $(ISODIR)
	mkdir -p $(ISODIR)/boot/grub
	cp $(KERNEL) $(ISODIR)/boot/kernel.bin
	cp boot/grub/grub.cfg $(ISODIR)/boot/grub/grub.cfg
	grub-mkrescue -o $@ $(ISODIR)

run: $(ISO)
	qemu-system-i386 -cdrom $(ISO) -m 512

run-kernel: $(KERNEL)
	qemu-system-i386 -kernel $(KERNEL)

clean:
	rm -rf $(BUILD_DIR)

re: clean all

.PHONY: all iso run run-kernel clean