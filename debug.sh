#!/bin/sh

BUILD_DIR=build
DISK_IMG="$BUILD_DIR/disk.img"

echo "Starting QEMU with debugging enabled..."
echo ""
echo "Useful QEMU monitor commands:"
echo "  Ctrl+Alt+2    - Switch to QEMU monitor"
echo "  Ctrl+Alt+1    - Switch back to console"
echo "  info registers - Show all CPU registers"
echo "  info gdt       - Show GDT"
echo "  info idt       - Show IDT"
echo "  x/10i \$eip    - Disassemble at current EIP"
echo ""

qemu-system-x86_64 \
    -kernel "$BUILD_DIR/kernel.bin" \
    -hda "$DISK_IMG" \
    -m 128M \
    -monitor stdio \
    -d int,cpu_reset \
    -no-reboot \
    -no-shutdown
