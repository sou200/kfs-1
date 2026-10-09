CC = gcc
NASM = nasm
LD = ld

CFLAGS  = -m32 -ffreestanding -O2 -Wall -Wextra
LDFLAGS = -m elf_i386 -T linker.ld

KERNEL = kernel.bin
ISO    = mykernel.iso

all: $(KERNEL)

boot.o: boot.asm
	$(NASM) -f elf32 $< -o $@

kernel.o: kernel.c
	$(CC) $(CFLAGS) -c $< -o $@

$(KERNEL): boot.o kernel.o linker.ld
	$(LD) $(LDFLAGS) -o $@ boot.o kernel.o

iso: $(ISO)

$(ISO): $(KERNEL) boot/grub/grub.cfg
	rm -rf isodir
	mkdir -p isodir/boot/grub
	cp $(KERNEL) isodir/boot/kernel.bin
	cp boot/grub/grub.cfg isodir/boot/grub/grub.cfg
	grub-mkrescue -o $@ isodir --compress=xz --install-modules="multiboot"

run: $(ISO)
	qemu-system-i386 -cdrom $(ISO) -m 512

run-kernel: $(KERNEL)
	qemu-system-i386 -kernel $(KERNEL)

clean:
	rm -rf *.o $(ISO) $(KERNEL) isodir

.PHONY: all iso run run-kernel clean