section .text

%define KSTACK_SIZE (4096)
%define CPU_NUM (1)

extern main
global kentry

kentry:
    mov rsp, _kstack + KSTACK_SIZE
    mov rbp, rsp
    jmp main

section .data

_kstack:    times KSTACK_SIZE * CPU_NUM db 00