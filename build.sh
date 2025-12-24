#!/bin/sh
set -e

BUILD_DIR=build
DISK_IMG="$BUILD_DIR/disk.img"
ARCH=i386

CC=gcc
LD=ld
AS=nasm

CFLAGS="-ffreestanding -nostdlib -fno-builtin -fno-stack-protector -Wall -Wextra -Werror -Iinclude -m32"
ASFLAGS="-f elf32"
LDFLAGS="-m elf_i386"

SRC_DIRS="src/kernel src/cpu src/drivers src/mm src/user src/syscall src/lib src/fs"

GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m'

echo "${BLUE}[*] Cleaning build directory${NC}"
rm -rf "$BUILD_DIR"/*.o

echo "${BLUE}[*] Creating build directory${NC}"
mkdir -p "$BUILD_DIR"

echo "${BLUE}[*] Assembling ASM files${NC}"
for dir in $SRC_DIRS; do
    if [ -d "$dir" ]; then
        for file in "$dir"/*.asm; do
            if [ -f "$file" ]; then
                obj="$BUILD_DIR/$(basename "$file" .asm).o"
                echo "    ${GREEN}AS${NC} $file"
                $AS $ASFLAGS "$file" -o "$obj"
            fi
        done
    fi
done

echo "${BLUE}[*] Compiling C files${NC}"
for dir in $SRC_DIRS; do
    if [ -d "$dir" ]; then
        for file in "$dir"/*.c; do
            if [ -f "$file" ]; then
                obj="$BUILD_DIR/$(basename "$file" .c).o"
                echo "    ${GREEN}CC${NC} $file"
                $CC $CFLAGS -c "$file" -o "$obj"
            fi
        done
    fi
done

echo "${BLUE}[*] Linking kernel${NC}"
$LD $LDFLAGS -T src/kernel/link.ld \
    "$BUILD_DIR"/*.o \
    -o "$BUILD_DIR/kernel.bin"

echo "${BLUE}[*] Creating disk image${NC}"
if [ ! -f "$DISK_IMG" ]; then
    echo "    Creating new 16MB disk: $DISK_IMG"
    dd if=/dev/zero of="$DISK_IMG" bs=1M count=16 2>/dev/null
else
    echo "    Using existing disk: $DISK_IMG"
fi

echo "${GREEN}[✓] Build complete!${NC}"
echo ""
echo "${BLUE}[*] Running QEMU with disk${NC}"
qemu-system-x86_64 \
    -kernel "$BUILD_DIR/kernel.bin" \
    -hda "$DISK_IMG" \
    -m 128M
