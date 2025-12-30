# QuantumKernel

A minimal 32-bit operating system kernel with basic system call support and an interactive shell.

## Features

- Protected mode with GDT and IDT
- Keyboard input handling
- VGA text mode terminal
- System call interface (int 0x80)

  
<img width="629" height="433" alt="image" src="https://github.com/user-attachments/assets/709dda66-77a6-41af-a6b0-0153cf0c8153" />




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


## System Calls

The kernel supports the following system calls (invoked via `int 0x80`):

| Number | Name            | Description                                      | Arguments                              | Return value |
|--------|-----------------|--------------------------------------------------|----------------------------------------|--------------|
| 1      | `write`         | Write data to file descriptor (stdout only)      | `fd`, `buf`, `len`                     | bytes written |
| 2      | `read`          | Read data from file descriptor (stdin only)      | `fd`, `buf`, `len`                     | bytes read   |
| 3      | `exit`          | Terminate the current process                    | —                                      | —            |
| 4      | `clear`         | Clear the screen                                 | —                                      | —            |
| 5      | `disk_read`     | Read raw sector from disk                        | `lba`, `buffer`                        | 0 on success |
| 6      | `disk_write`    | Write raw sector to disk                         | `lba`, `buffer`                        | 0 on success |
| 7      | `sleep`         | Sleep for specified milliseconds                 | `ms`                                   | —            |
| 8      | `open`          | Open file on FAT32 filesystem                    | `path`, `flags`                        | file descriptor (>=0) or -1 |
| 9      | `close`         | Close file descriptor                            | `fd`                                   | 0 on success |
| 10     | `file_read`     | Read from opened file                            | `fd`, `buffer`, `size`                 | bytes read   |
| 11     | `file_write`    | Write to opened file                             | `fd`, `buffer`, `size`                 | bytes written|
| 12     | `seek`          | Change file position                             | `fd`, `offset`, `whence`               | 0 on success |
| 13     | `unlink`        | Delete file                                      | `path`                                 | 0 on success |
| 14     | `mkdir`         | Create directory                                 | `path`                                 | 0 on success |
| 15     | `readdir`       | Read next directory entry from root/cluster      | `cluster`, `index` (in/out), `info`    | 0 on success |
| 16     | `rtc_time`      | Get current local time from RTC                  | `rtc_time_t* out`                      | —            |
| 17     | `timezone`      | Get current timezone offset (hours from UTC)      | —                                      | offset       |

System calls follow the standard i386 calling convention:
- Syscall number in `eax`
- Arguments in `ebx`, `ecx`, `edx` (up to 3)
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

