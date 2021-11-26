; Warning: Now loader is 502 bytes in size, remember to modify Makefile if ADD ANYTHING!!

%include "asm.inc"

section .text

global _start
global map_kernel

extern loader_c_entry

_start:
[bits 64]
    jmp loader_c_entry

map_kernel:
    push rax
    push rdi
    push r9

    mov rdi,PML4
    mov r9,[rdi]
    mov rax,0xFF0 ; 0xFFFFFF0000000000 bits [47:39] << 3
    add rdi,rax

    ; reuse same PML4E because both 2 mapping virtual address bits [38:30] equals to 0.
    mov [rdi],r9
    
    pop r9
    pop rdi
    pop rax

