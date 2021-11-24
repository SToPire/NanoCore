section .text

global _start
extern loader_c_entry

_start:
[bits 64]
    jmp loader_c_entry