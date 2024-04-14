section .text

global exec
exec:
    mov rax, 1
    int 0x80
    ret
global write
write:
    mov rax, 2
    int 0x80
    ret
global read
read:
    mov rax, 3
    int 0x80
    ret
