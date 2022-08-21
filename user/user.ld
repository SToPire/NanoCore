SECTIONS {
  .text 0x100000 : {
    *(.text .stub .text.* .gnu.linkonce.t.*)
    *(.rodata .rodata.* .gnu.linkonce.r.*)
  }

  .data : {
    *(.data)
  }

  /DISCARD/ : {
      *(.eh_frame .note.gnu.* .comment)
  }
}