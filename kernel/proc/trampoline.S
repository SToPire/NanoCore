section .text

global ret_to_user
global iret_from_trapframe

; %rsp must point to struct trapframe
iret_from_trapframe:
    ; get ss, user it to recover other seg reg
    ; ss itself will be recover in iretq
    mov rax, [rsp + (0x8 * (15 + 2 + 4))]
    mov ds, rax
    mov es, rax
    mov fs, rax
    mov gs, rax

    pop rax
    pop rbx
    pop rcx
    pop rdx
    pop rdi
    pop rsi
    pop rbp
    pop r8
    pop r9
    pop r10
    pop r11
    pop r12
    pop r13
    pop r14
    pop r15

    add rsp, 0x8 ; skip trapno
    add rsp, 0x8 ; skip errno

    iretq

; arg1 (in %rdi): %rsp pointer
ret_to_user:
  mov rsp, rdi
  jmp iret_from_trapframe
