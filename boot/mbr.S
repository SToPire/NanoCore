section MBR vstart=0x7c00

real_mode:
[bits 16]
    xor ax,ax
    ; DO NOT try to 'mov cs,ax' as 'mov' instruction cannot change the workflow. 
    ; It is invalid and will cause an invalid opcode exception(#UD).
    mov ds,ax
    mov es,ax
    mov fs,ax
    mov gs,ax
    mov ss,ax
    mov sp,0x7c00

    ; enable A20 line, ref to https://wiki.osdev.org/A20_Line
    in al,0x92
    test al,2
    jnz seta20end
    or al,2
    and al,0xFE
    out 0x92,al
    seta20end:

    ; load GDT
    lgdt [GDT_DESC]

    ; enable protected mode
    mov eax,cr0
    or eax,0x1
    mov cr0,eax

    ; use long jump to reload $CS with selector which selects the code segment
    jmp (1<<3):protected_mode

protected_mode:
[bits 32]
    mov ax,(2<<3) ;set other seg reg to the data segment
    mov ds,ax
    mov es,ax
    mov fs,ax
    mov gs,ax
    mov ss,ax

    jmp $


; Global Descriptor Table, ref to https://wiki.osdev.org/Global_Descriptor_Table
align 8
GDT_BASE: 
    ; gdt 0: null entry
    dd 0x00000000
    dd 0x00000000

    ; gdt 1: code seg
    dd 0x0000FFFF
    db 0x00,0x9A,0xCF,0x00

    ; gdt 2: data seg
    dd 0x0000FFFF
    db 0x00,0x92,0xCF,0x00

GDT_DESC: ; GDT Descriptor will be loaded into GDTR
    dw GDT_DESC-GDT_BASE-1  ; first 2 bytes: GDT_LIMIT = sizeof(GDT) - 1
    dd GDT_BASE             ; next 4 bytes: GDT_BASE_ADDR


; valid MBR must be 512 bytes in size and end with magic number
times 510-($-$$) db 0
db 0x55,0xaa