NAME = kernel.bin
ISO = kfs1.iso

CC = gcc
CFLAGS = -m32 -fno-builtin -fno-exceptions -fno-stack-protector -nostdlib -nodefaultlibs -Wall -Wextra -Werror
AS = nasm
ASFLAGS = -f elf32
LD = ld
LDFLAGS = -m elf_i386 -T linker.ld

# ADD YOUR NEW FILES HERE
#
OBJS = boot.o printk.o kernel.o terminal.o keyboard.o vga_cursor.o

all: $(ISO)

%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(NAME): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(NAME)

$(ISO): $(NAME)
	mkdir -p isodir/boot/grub
	cp $(NAME) isodir/boot/$(NAME)
	cp grub.cfg isodir/boot/grub/grub.cfg
	# Using the local path fix from earlier
	PATH="$$HOME/.local/usr/bin:$$PATH" grub-mkrescue -d $$HOME/.local/usr/lib/grub/i386-pc -o $(ISO) isodir

debug: $(NAME)
	qemu-system-i386 -kernel $(NAME)

clean:
	rm -rf $(OBJS) isodir

fclean: clean
	rm -f $(NAME) $(ISO)

re: fclean all
