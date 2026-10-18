# KFS-1: Kernel From Scratch - Boot, Screen, and I/O

Welcome to **KFS-1**, the first step in the 42 Kernel From Scratch branch. This project implements a 32-bit (`i386`) bare-metal kernel capable of booting via GRUB, managing the physical VGA text buffer, handling PS/2 keyboard input, and supporting multiple virtual terminals.

Because this is a bare-metal environment compiled with `-nostdlib` and `-fno-builtin`, there is no underlying Operating System. There is no `libc`, no `malloc`, and no standard error handling (like `errno`). Memory is accessed via direct physical pointers, and hardware is controlled via CPU Port-Mapped I/O (PMIO).

---

## 🛠 Compilation and Usage

**Requirements:** `nasm`, `gcc` (multilib for 32-bit), `ld`, `grub-mkrescue`, `xorriso`, and `qemu-system-i386`.

*   **`make`**: Compiles the source code and generates the bootable `kfs1.iso`.
*   **`make debug`**: Bypasses the ISO generation and directly boots `kernel.bin` in QEMU for rapid testing.
*   **`make clean` / `make fclean`**: Removes object files and binaries.
*   **`make re`**: Recompiles the entire project.

Once booted, you can type on the screen and use **F1, F2, F3, and F4** to switch between four independent virtual terminals.

---

## 📚 Detailed Function Reference

In a standard C program, functions often return `-1` or `NULL` on failure. **In kernel development, "failure" usually means undefined hardware behavior, a CPU exception, or a triple-fault (reboot).** Functions here generally return `void` or the exact data requested, utilizing early-returns to prevent logic errors (like out-of-bounds array access).

### 1. I/O Port Communication (`io.h`)
The x86 architecture uses separate instructions (`in`, `out`) to communicate with hardware peripherals. These inline assembly functions wrap those instructions.

*   **`static inline void outb(uint16_t port, uint8_t val)`**
    *   **Description:** Writes a single byte (`val`) to the specified 16-bit hardware `port`.
    *   **Returns:** `void`.
    *   **Failure State:** Writing to the wrong port or sending invalid data will cause the target hardware peripheral to malfunction or the kernel to crash.

*   **`static inline uint8_t inb(uint16_t port)`**
    *   **Description:** Reads a single byte from the specified hardware `port`.
    *   **Returns:** The `uint8_t` (8-bit) value provided by the hardware.
    *   **Failure State:** Reading from an uninitialized or wrong port typically returns garbage data (often `0xFF`).

### 2. VGA Hardware Cursor (`vga_cursor.c`)
Controls the blinking hardware cursor via the VGA controller's multiplexed ports (`0x3D4` and `0x3D5`).

*   **`void enable_cursor(uint8_t cursor_start, uint8_t cursor_end)`**
    *   **Description:** Configures the VGA registers to enable the cursor and sets its thickness (scanlines).
    *   **Parameters:** `cursor_start` (usually 14), `cursor_end` (usually 15).
    *   **Returns:** `void`.

*   **`void update_cursor(int x, int y)`**
    *   **Description:** Calculates the linear 16-bit offset of the `(x, y)` coordinates and sends it in two 8-bit chunks (Low Byte, High Byte) to the VGA controller.
    *   **Returns:** `void`.

### 3. Terminal and Screen Driver (`terminal.c`)
Manages the `0xB8000` memory map, virtual screens, colors, and scrolling.

*   **`static inline uint16_t vga_entry(unsigned char uc, uint8_t color)`**
    *   **Description:** Bitwise combines an 8-bit ASCII character and an 8-bit color code into the 16-bit format required by the VGA hardware.
    *   **Returns:** `uint16_t` formatted correctly for memory mapping.

*   **`void terminal_initialize(void)`**
    *   **Description:** Sets the default color (White on Black), clears the 4 hidden virtual screen buffers, and wipes the physical VGA memory to remove BIOS artifacts. Resets cursor coordinates to `(0,0)`.
    *   **Returns:** `void`.

*   **`void terminal_switch(int new_screen)`**
    *   **Description:** Saves the current physical VGA memory to the active hidden buffer, then copies the requested `new_screen` buffer into physical memory.
    *   **Returns:** `void`.
    *   **Failure Handling:** If `new_screen` is out of bounds (`< 0` or `>= NUM_SCREENS`), or if the user requests the currently active screen, the function returns early without doing anything to prevent memory corruption.

*   **`static void terminal_scroll(void)`**
    *   **Description:** Shifts rows 1 through 24 up by one row in memory, overwriting row 0. Then, clears row 24 with spaces to allow new text.
    *   **Returns:** `void`.

*   **`void terminal_putchar(char c)`**
    *   **Description:** Processes a single character. If `\n`, it increments the row. Otherwise, it places the character into the VGA buffer and increments the column. Triggers `terminal_scroll()` if the bottom of the screen is reached. Finally, calls `update_cursor()`.
    *   **Returns:** `void`.

*   **`void terminal_writestring(const char* data)`**
    *   **Description:** Iterates through a null-terminated string, passing each character to `terminal_putchar`.
    *   **Returns:** `void`.

### 4. Custom Formatting (`printk.c`)
A lightweight replacement for the standard `printf` function, utilizing GCC `__builtin_va_list` macros since `<stdarg.h>` is unavailable.

*   **`static void print_itoa(int num)`**
    *   **Description:** Converts a base-10 signed integer into ASCII characters and prints them. Handles negative numbers.
    *   **Returns:** `void`.

*   **`static void print_hex(unsigned int num)`**
    *   **Description:** Converts an unsigned integer into a base-16 hexadecimal string (lowercase) and prints it.
    *   **Returns:** `void`.

*   **`void printk(const char* format, ...)`**
    *   **Description:** Parses a format string. Supports `%c` (char), `%s` (string), `%d` (signed int), and `%x` (hexadecimal). Dispatches to the appropriate helper function.
    *   **Returns:** `void`. *(Note: Standard `printf` returns the number of characters printed. To keep this kernel minimal, `printk` does not calculate this).*
    *   **Failure Handling:** If an unknown format specifier is passed (e.g., `%q`), the parser gracefully falls back to printing the `%` and the unknown character literally.

### 5. PS/2 Keyboard Driver (`keyboard.c`)
Polls the Intel 8042 PS/2 controller for hardware interrupts and translates scan codes into ASCII characters.

*   **`uint8_t read_scancode(void)`**
    *   **Description:** Continuously reads the status port (`0x64`). Once bit `0` indicates the output buffer is full, it reads and returns the data from port (`0x60`).
    *   **Returns:** `uint8_t` hardware scan code.
    *   **Failure Handling:** This is a "blocking" function. It implements a busy-wait loop (`while(1)`). If the keyboard hardware dies, this loop will halt the kernel indefinitely.

*   **`void keyboard_loop(void)`**
    *   **Description:** The main infinite loop of the kernel. It fetches scan codes, discards key-release events (codes with bit 7 set), intercepts F1-F4 keys to trigger `terminal_switch()`, maps remaining codes through the `kbd_azerty` array, and passes valid ASCII characters to `terminal_putchar()`.
    *   **Returns:** Does not return (infinite loop).

### 6. Main Entry Point (`kernel.c`)

*   **`void kmain(void)`**
    *   **Description:** The C entry point called by `boot.s` after the stack is established. It initializes the terminal, enables the cursor, prints the mandatory "42" header utilizing `printk`, and hands execution over to `keyboard_loop()`.
    *   **Returns:** `void`. Under normal circumstances, execution never reaches the end of this function.