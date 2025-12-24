OUTPUT_FORMAT(elf32-i386)
ENTRY(_start)

SECTIONS {
    . = 1M;  /* Load kernel at 1MB (GRUB standard) */

    .text : { *(.multiboot) *(.text) }
    .data : { *(.data) }
    .bss  : { *(.bss) }
}
