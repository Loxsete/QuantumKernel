#!/bin/sh

BUILD_DIR=build
ISO_IMG="$BUILD_DIR/QuantumKernel.iso"
DISK_IMG="$BUILD_DIR/disk.img"

echo "Starting QEMU with debugging enabled..."
echo ""
echo "Useful QEMU monitor commands:"
echo " Ctrl+Alt+2  - QEMU monitor"
echo " Ctrl+Alt+1  - Back to VGA"
echo " info registers"
echo " info gdt"
echo " info idt"
echo " x/10i $pc"
echo ""

qemu-system-x86_64 \
    -vga std \
    -boot d \
    -cdrom "$ISO_IMG" \
    -hda "$DISK_IMG" \
    -m 128M \
    -monitor stdio \
    -d int,cpu_reset \
    -no-reboot \
    -no-shutdown \
    -rtc base=localtime,clock=host
