#!/bin/sh
set -e

BUILD_DIR=build
ARCH=i386

CC=gcc
LD=ld
AS=nasm

CFLAGS="-ffreestanding -nostdlib -fno-builtin -fno-stack-protector -Wall -Wextra -Werror -Iinclude -m32"
ASFLAGS="-f elf32"
LDFLAGS="-m elf_i386"

echo "[*] Creating build dir"
mkdir -p "$BUILD_DIR"

echo "[*] Assembling ASM files"
for file in src/*.asm; do
    obj="$BUILD_DIR/$(basename "$file" .asm).o"
    echo "    AS $file"
    $AS $ASFLAGS "$file" -o "$obj"
done


echo "[*] Compiling C files"
for file in src/*.c; do
    obj="$BUILD_DIR/$(basename "$file" .c).o"
    echo "    CC $file"
    $CC $CFLAGS -c "$file" -o "$obj"
done

echo "[*] Linking kernel"
$LD $LDFLAGS -T src/link.ld \
    "$BUILD_DIR"/*.o \
    -o "$BUILD_DIR/kernel.bin"


echo "[*] Done!"
echo "[*] Running QEMU"

qemu-system-x86_64 -kernel "$BUILD_DIR/kernel.bin"
