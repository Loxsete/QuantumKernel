# QuantumKernel

A minimal 32-bit operating system kernel with basic system call support and an interactive shell.

## Features

- Protected mode with GDT and IDT
- Keyboard input handling
- VGA text mode terminal
- System call interface (int 0x80)

  
<img width="646" height="546" alt="screen" src="https://github.com/user-attachments/assets/09604d76-3469-4f5a-a380-b64e99d7a12b" />




## Prerequisites

You'll need the following tools installed:

- GCC with 32-bit support (`gcc-multilib` on Ubuntu/Debian)
- GNU ld linker
- NASM assembler
- QEMU (for testing)

### Installing dependencies

**Ubuntu/Debian:**
```bash
sudo apt install build-essential nasm qemu-system-x86 gcc-multilib
```

**Arch Linux:**
```bash
sudo pacman -S base-devel nasm qemu-system-x86 lib32-gcc-libs
```

**macOS:**
```bash
brew install nasm qemu i686-elf-gcc
```
Note: On macOS, you may need to adjust the build script to use `i686-elf-gcc` instead of `gcc`.

## Building

The project includes a simple build script that compiles all source files and creates a bootable kernel binary.

```bash
chmod +x build.sh
./build.sh
```

This will:
1. Create a `build/` directory
2. Assemble all `.asm` files
3. Compile all `.c` files with 32-bit flags
4. Link everything into `build/kernel.bin`
5. Launch QEMU automatically


## Running

### With QEMU (automatic)

The build script runs QEMU automatically after compilation. You can also run it manually:

```bash
qemu-system-x86_64 -kernel build/kernel.bin
```

### With VirtualBox or VMware

To run on real virtualization software, you'll need to create a bootable ISO. This requires GRUB:

```bash
chmod +x iso.sh
./iso.sh
```

Then boot `myos.iso` in your VM software.



## System Calls

The kernel supports the following system calls (invoked via `int 0x80`):

| Number | Name      | Description                    |
|--------|-----------|--------------------------------|
| 1      | exit      | Terminate process              |
| 2      | write     | Write to file descriptor       |
| 3      | exit      | Terminates the current process |
| 4      | clear     | Clears screen                  |
| 5      | disk_read | Read disk sector               |
| 6      | disk_write| Write disk sector              |
| 7      | sleep     | Wait milliseconds              |


System calls follow the standard i386 convention:
- `eax` = syscall number
- `ebx`, `ecx`, `edx` = arguments
- Return value in `eax`

## Debugging

To debug with GDB:

```bash
qemu-system-x86_64 -kernel build/kernel.bin -s -S
```

In another terminal:
```bash
gdb build/kernel.bin
(gdb) target remote localhost:1234
(gdb) break kernel_main
(gdb) continue
```

## Contributing

This is a learning project, but improvements are welcome! Areas for contribution:

- Virtual filesystem implementation
- Process/thread scheduling
- User mode support
- More device drivers (timer, disk, etc.)
- Memory paging

