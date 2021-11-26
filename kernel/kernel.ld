ENTRY(_start)

SECTIONS
{
    OFFSET = 0xFFFFFF0000000000;
    KERNEL_START = OFFSET + 0x100000;

    . = KERNEL_START;

    .text : AT(ADDR(.text) - OFFSET) {
        *(.text .stub .text.* .gnu.linkonce.t.*)
        *(.rodata .rodata.* .gnu.linkonce.r.*)
        . = ALIGN(0x1000);
    }

    PROVIDE(etext = .);

    .data : AT(ADDR(.data) - OFFSET) {
        *(.data)
        . = ALIGN(0x1000);
    }

    PROVIDE(edata = .);

    .bss : AT(ADDR(.bss) - OFFSET) {
        *(.bss)
        . = ALIGN(0x1000);
    }

    PROVIDE(end = .);

    /DISCARD/ : {
        *(.eh_frame .note.gnu.* .comment)
    }
}