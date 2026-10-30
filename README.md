# KFS_2: GDT and Stack

This repository implements the 42 KFS_2 - GDT & Stack subject. The subject requires a 32-bit i386 kernel with a Global Descriptor Table at physical address `0x00000800`, followed by a human-readable kernel stack dump. The interactive shell, `reboot`, and `halt` commands are bonus functionality.

## Subject Requirements

### Mandatory part

The mandatory implementation must:

- Create and load a Global Descriptor Table.
- Include these six usable segments: kernel code, kernel data, kernel stack, user code, user data, and user stack.
- Include the required null descriptor.
- Place the GDT at `0x00000800`.
- Declare the GDT to the processor with `lgdt`.
- Print the kernel stack in a human-readable format.
- Stay within the subject's 10 MB repository-size limit.
- Include a `Makefile`, kernel source, and a bootable virtual image.

### Bonus part

The bonus is evaluated only after the mandatory part is complete and works without malfunctioning. This repository includes a debugging shell with `stack`, `stack N`, `reboot`, and `halt` commands. Keyboard input, VGA terminal switching, and color changing also support the shell demonstration.

## Repository Layout

```text
boot/grub/grub.cfg   GRUB Multiboot configuration
include/             Kernel headers
src/boot.asm         Multiboot header, entry point, bootstrap stack
src/gdt.c            GDT descriptors and stack dump
src/gdt_flush.asm    lgdt and segment-register reload
src/kernel.c         Kernel initialization and idle loop
src/vga.c            VGA output and printk formatter
src/keyboard.c       PS/2 scancode translation and keyboard dispatch
src/cmd.c            Command line buffer and shell commands
src/idt.c             Interrupt Descriptor Table
src/isr.c             C interrupt dispatcher
src/isr_stub.asm      IRQ entry stubs and iret path
src/io.c              Port I/O and PIC remapping
linker.ld             Kernel memory layout
Makefile              Build, ISO, and QEMU targets
```

## Toolchain and Architecture

The subject requires i386. The project uses GCC for freestanding 32-bit C, NASM for 32-bit assembly, GNU `ld` with the project-owned `linker.ld`, GRUB Multiboot, and QEMU.

The kernel does not use a host operating-system library. It has no libc, dynamic loader, or runtime allocator. Functions normally provided by libc, such as formatted output and string comparison, are implemented in the kernel.

The Makefile deliberately uses the subject's freestanding flags:

```text
-m32 -ffreestanding -fno-builtin -fno-stack-protector
-nostdlib -nodefaultlibs
```

`-m32` targets i386. `-ffreestanding` removes hosted-library assumptions. `-fno-builtin` prevents implicit libc builtins. `-fno-stack-protector` avoids references to a stack-canary runtime that the kernel does not provide. `-nostdlib` and `-nodefaultlibs` prevent host startup files and libraries from being linked.

The host `ld` binary is permitted by the subject; the linker script is this repository's `linker.ld`, not a host linker script.

## Build and Run

Install GCC with 32-bit support, NASM, GRUB rescue tools, QEMU, and xorriso. Then run:

```sh
make clean all
make run
```

Build artifacts are:

- `build/kernel.bin`: linked ELF32 kernel.
- `build/mykernel.iso`: bootable GRUB ISO.

Other targets:

```sh
make run-kernel   # Boot kernel.bin directly with QEMU
make clean        # Remove build/
make re           # Clean and rebuild everything
```

The normal evaluation image is `build/mykernel.iso`, containing `/boot/kernel.bin` and `/boot/grub/grub.cfg`.

## Boot Sequence

1. GRUB reads the Multiboot header in `src/boot.asm`.
2. GRUB loads the kernel according to `boot/grub/grub.cfg`.
3. `_start` sets `ESP` and `EBP` to the top of the 16 KiB bootstrap stack.
4. `_start` calls `kernel_main`.
5. `kernel_main` disables interrupts with `cli`.
6. `init_terminals()` initializes VGA state.
7. `pic_remap(0x20, 0x28)` moves hardware IRQs away from CPU exception vectors.
8. `init_gdt()` fills the GDT at `0x00000800` and loads it.
9. `idt_init()` installs the timer and keyboard interrupt gates.
10. `sti` enables maskable interrupts.
11. The welcome message and shell prompt are printed.
12. The kernel enters an infinite `hlt` loop until an interrupt arrives.

The kernel starts in 32-bit protected mode as loaded by GRUB. The GDT is required because protected-mode segment registers must refer to valid descriptors before the kernel can safely use its intended code, data, and stack segments.

## GDT Implementation

### Descriptor format

An x86 GDT is an array of eight-byte descriptors. Each descriptor contains a 32-bit segment base, a 20-bit segment limit, access flags, privilege level, executable/readable or writable bits, granularity, and operand-size information.

`src/gdt.c` defines packed `gdt_entry` and `gdt_ptr` structures so their memory layout matches the CPU format. `gdt_set_gate()` splits the base and limit into the descriptor's low, middle, and high fields.

### Required GDT location

The subject explicitly requires the GDT at `0x00000800`. This implementation writes entries through:

```c
struct gdt_entry *gdt = (struct gdt_entry *)0x00000800;
```

The table has seven entries, including the null entry:

```text
7 entries * 8 bytes = 56 bytes
limit = 56 - 1 = 55
base  = 0x00000800
```

The hard-coded address is intentional for KFS_2 and must not be changed for this assignment.

### Descriptor table

| Entry | Selector | Segment | Base | Limit | Access | Granularity |
| ---: | ---: | --- | ---: | ---: | ---: | ---: |
| 0 | `0x00` | Null | `0x00000000` | `0x00000000` | `0x00` | `0x00` |
| 1 | `0x08` | Kernel code | `0x00000000` | `0xFFFFFFFF` | `0x9A` | `0xCF` |
| 2 | `0x10` | Kernel data | `0x00000000` | `0xFFFFFFFF` | `0x92` | `0xCF` |
| 3 | `0x18` | Kernel stack | `0x00000000` | `0xFFFFFFFF` | `0x96` | `0xCF` |
| 4 | `0x20` | User code | `0x00000000` | `0xFFFFFFFF` | `0xFA` | `0xCF` |
| 5 | `0x28` | User data | `0x00000000` | `0xFFFFFFFF` | `0xF2` | `0xCF` |
| 6 | `0x30` | User stack | `0x00000000` | `0xFFFFFFFF` | `0xF6` | `0xCF` |

All usable entries are flat 32-bit segments. Their base is zero and their limit covers the 32-bit address space. The table defines the required segment types and privileges; it does not yet create paging-based memory isolation or enable user mode.

The selector is the descriptor index multiplied by eight: kernel code `0x08`, kernel data `0x10`, kernel stack `0x18`, user code `0x20`, user data `0x28`, and user stack `0x30`.

### Loading the GDT

`init_gdt()` creates a `gdt_ptr` and calls the external assembly function `gdt_flush()` from `src/gdt_flush.asm`.

`gdt_flush()`:

1. Reads the pointer argument from `[esp+4]`.
2. Executes `lgdt [eax]` to load the processor's GDTR.
3. Loads selector `0x10` into `DS`, `ES`, `FS`, and `GS`.
4. Loads selector `0x18` into `SS`.
5. Performs a far jump to `0x08:.flush` to reload `CS` with kernel code.
6. Returns to the C caller.

The far jump is required because loading GDTR alone does not reload the cached code-segment descriptor in `CS`.

### GDT verification

For the mandatory defense, verify that the table base is exactly `0x00000800`, the limit is exactly `55`, entry zero is null, entries 1 through 6 match the required roles, `CS=0x08`, data segments use `0x10`, and `SS=0x18` after `gdt_flush()`.

The GDT is initialized while interrupts are disabled, preventing an IRQ from using partially initialized segment state.

## Kernel Stack and Stack Dump

### Bootstrap stack

`src/boot.asm` reserves 16 KiB in `.bss`:

```asm
stack_bottom:
    resb 16384
stack_top:
```

The stack grows downward on i386. `_start` initializes both `ESP` and `EBP` to `stack_top`. A C call pushes a return address and creates stack frames below that address. `push` decreases `ESP`; `pop` increases it.

The GDT's kernel-stack descriptor is selector `0x18`. It is a flat writable ring-0 data descriptor, so it describes the address range used by the kernel stack. It does not allocate or check stack storage; the actual storage is the 16 KiB `.bss` reservation.

### `print_stack()`

`print_stack()` in `src/gdt.c` reads `ESP` and `EBP` with inline assembly, prints both registers, then prints each 32-bit word while moving from `ESP` toward `EBP`. The output is hexadecimal and human-readable:

```text
--- Kernel Stack Dump ---
ESP: 0x0010f000 | EBP: 0x0010f080
[0x0010f000]: 0x00000000
[0x0010f004]: 0x00100123
-------------------------
```

This is a diagnostic dump, not a complete symbolic backtrace. The walk is bounded by the real `stack_bottom`/`stack_top` interval and by 256 words, but it still depends on a conventional frame layout and is not symbolic.

## `printk` Implementation

The console formatter is exposed as `printk()` in `include/screen.h` and implemented in `src/vga.c`. Its reusable formatting core is `vprintk()`, which accepts an already-open `va_list`.

It writes to VGA text memory at `0xB8000`. Each cell contains a character in the low byte and a color attribute in the high byte. `vga_putchar()` handles newlines, wrapping, scrolling, and the hardware cursor. `printk()` formats text without libc using `stdarg.h`.

Supported format specifiers:

| Specifier | Argument | Result |
| --- | --- | --- |
| `%c` | promoted `int` | One character |
| `%s` | `const char *` | String, or `(null)` |
| `%d` | `int` | Signed decimal |
| `%x` | `uint32_t` | Lowercase hexadecimal |
| `%%` | none | A literal percent sign |

The formatter is used by the boot message, shell errors, reboot/halt messages, and `print_stack()`.

The implementation follows `printk(format, ...) -> vprintk(format, va_list) -> VGA output`, allowing other logging wrappers to reuse the open argument list. Full libc `printf` compatibility is unnecessary for this subject.

## Bonus Shell

### Keyboard path

```text
PS/2 keyboard
  -> IRQ1 / interrupt vector 33
  -> IRQ1 in src/isr_stub.asm
  -> IRQ_handler(33)
  -> keyboard_handler()
  -> scancode_to_char()
  -> handle_keypress()
```

`keyboard_handler()` reads port `0x60`, ignores release scancodes, translates make scancodes, and sends characters to `handle_keypress()`. F1-F4 switch terminal snapshots and F5 changes the foreground color.

### Command buffer

`src/cmd.c` contains a 256-byte `cmd_buffer` and a `cmd_index`:

- Characters are echoed and stored while space remains.
- Enter writes the terminating null byte and calls `execute_command()`.
- Backspace removes one stored character and erases one VGA cell.
- After a command, the index is reset and `kfs-2> ` is printed.

The parser trims leading and trailing spaces and tabs before exact string comparison. Plain `stack` prints 12 words so the result remains readable on the 80x25 VGA screen. `stack N` requests N words, capped at 256, and reports omitted words. The parser also accepts `halt` and `reboot`.

### `halt`

The command prints a message and executes `cli` followed by `hlt`. Disabling maskable interrupts means keyboard input cannot wake the CPU, so the command is intentionally non-returning.

### `reboot`

The command polls the PS/2 controller status port `0x64` until the input buffer is clear, then writes `0xFE` to the same port. The wait is bounded so a broken controller cannot hang the shell forever. On compatible hardware and QEMU this requests a CPU reset; a future implementation can add an ACPI fallback.

## Known Follow-up Work

These items are outside the mandatory KFS_2 requirement but matter for robustness:

- Remove duplicated terminal cursor declarations in `src/vga.c`.
- Add a TSS before implementing ring-3 execution.

These do not change the subject's required GDT address. The GDT must remain at `0x00000800` for this assignment.

## Defense Checklist

Before submission or peer evaluation:

```sh
make clean all
```

Confirm that the command exits successfully, `build/kernel.bin` exists, `build/mykernel.iso` exists, the ISO boots in QEMU, the GDT is at `0x00000800`, all seven descriptors are correctly encoded, the stack dump is readable, no host libc or runtime library is linked, and the repository remains below 10 MB.

For the bonus demonstration, boot the ISO and test `stack`, `halt`, `reboot`, F1-F4, F5, ordinary typing, Enter, and backspace.
