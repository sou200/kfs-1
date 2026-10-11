CC   = gcc
NASM = nasm
LD   = ld

CFLAGS    = -m32 -ffreestanding -O2 -Wall -Wextra
NASMFLAGS = -f elf32
LDFLAGS   = -m elf_i386 -T linker.ld

KERNEL = kernel.bin
ISO    = mykernel.iso

ASM_SRCS = boot.asm isr_stub.asm
C_SRCS = kernel.c vga.c io.c idt.c isr.c

ASM_OBJS = $(ASM_SRCS:.asm=.o)
C_OBJS   = $(C_SRCS:.c=.o)
OBJS     = $(ASM_OBJS) $(C_OBJS)

all: $(KERNEL) iso

$(KERNEL): $(OBJS) linker.ld
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.asm
	$(NASM) $(NASMFLAGS) $< -o $@

iso: $(ISO)

$(ISO): $(KERNEL) boot/grub/grub.cfg
	rm -rf isodir
	mkdir -p isodir/boot/grub
	cp $(KERNEL) isodir/boot/kernel.bin
	cp boot/grub/grub.cfg isodir/boot/grub/grub.cfg
	grub-mkrescue -o $@ isodir

run: $(ISO)
	qemu-system-i386 -cdrom $(ISO) -m 512

run-kernel: $(KERNEL)
	qemu-system-i386 -kernel $(KERNEL)

clean:
	rm -rf $(OBJS) $(ISO) $(KERNEL) isodir


.PHONY: all iso run run-kernel clean