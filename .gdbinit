target remote localhost:6789

add-symbol-file build/loader.o
add-symbol-file build/mbr.o
add-symbol-file build/kernel

layout asm
layout reg
