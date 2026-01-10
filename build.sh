#!/bin/sh
set -e

BUILD_DIR=build
ISO_DIR=iso
DISK_IMG="$BUILD_DIR/disk.img"
ISO_IMG="$BUILD_DIR/QuantumKernel.iso"

ARCH=i386
CC=gcc
LD=ld
AS=nasm

CFLAGS="-ffreestanding -nostdlib -fno-builtin -fno-stack-protector -Wall -Wextra -Werror -Iinclude -m32"
ASFLAGS="-f elf32"
LDFLAGS="-m elf_i386"

SRC_DIRS="src/kernel src/cpu src/drivers src/mm src/user src/syscall src/lib src/fs src/errors src/drivers/pci src/drivers/rtl8139 src/drivers/net"

GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo "${BLUE}[*] Cleaning${NC}"
rm -rf "$BUILD_DIR" "$ISO_DIR"
mkdir -p "$BUILD_DIR" "$ISO_DIR"

echo "${BLUE}[*] Assembling ASM${NC}"
for dir in $SRC_DIRS; do
    [ -d "$dir" ] || continue
    for file in "$dir"/*.asm; do
        [ -f "$file" ] || continue
        obj="$BUILD_DIR/$(basename "$file" .asm).o"
        echo "    ${GREEN}AS${NC} $file"
        $AS $ASFLAGS "$file" -o "$obj"
    done
done

echo "${BLUE}[*] Compiling C${NC}"
for dir in $SRC_DIRS; do
    [ -d "$dir" ] || continue
    for file in "$dir"/*.c; do
        [ -f "$file" ] || continue
        obj="$BUILD_DIR/$(basename "$file" .c).o"
        echo "    ${GREEN}CC${NC} $file"
        $CC $CFLAGS -c "$file" -o "$obj"
    done
done

echo "${BLUE}[*] Linking kernel ELF${NC}"
$LD $LDFLAGS -T src/kernel/link.ld \
    "$BUILD_DIR"/*.o \
    -o "$BUILD_DIR/kernel.elf"

echo "${BLUE}[*] Creating FAT32 disk${NC}"
dd if=/dev/zero of="$DISK_IMG" bs=1M count=16 2>/dev/null
mkfs.fat -F 32 "$DISK_IMG"

echo "${BLUE}[*] Writing files to FAT32${NC}"
echo "UTC+3" > "$BUILD_DIR/tz.txt"
mcopy -i "$DISK_IMG" "$BUILD_DIR/tz.txt" ::tz.txt

echo "${BLUE}[*] Preparing GRUB ISO${NC}"
mkdir -p "$ISO_DIR/boot/grub"
cp "$BUILD_DIR/kernel.elf" "$ISO_DIR/boot/kernel.elf"

cat > "$ISO_DIR/boot/grub/grub.cfg" << EOF
set timeout=0
set default=0

menuentry "QuantumKernel" {
    multiboot /boot/kernel.elf
    set gfxpayload=1024x768x32
    boot
}
EOF

grub-mkrescue -o "$ISO_IMG" "$ISO_DIR" >/dev/null 2>&1

echo "${GREEN}[✓] Build complete${NC}"
echo "${BLUE}[*] ISO: $ISO_IMG${NC}"
echo "${BLUE}[*] Disk: $DISK_IMG${NC}"
echo ""

echo "${BLUE}[*] Running QEMU${NC}"
qemu-system-x86_64 \
    -vga std \
    -boot d \
    -cdrom "$ISO_IMG" \
    -hda "$DISK_IMG" \
    -m 128M \
    -netdev user,id=net0 \
    -device rtl8139,netdev=net0 \
    -no-reboot \
    -no-shutdown \
    -rtc base=localtime,clock=host
