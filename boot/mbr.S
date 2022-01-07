; Warning: MBR's size is close to its upper bound(510 bytes), DO NOT ADD ANYTHING!!

%include "asm.inc"

section .text

global _start ; ld use this name as entry point
extern mbr_c_entry

_start:
[bits 16]
    cli ; disable interrupts enabled by BIOS

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

    ; load GDT32 for protected mode
    lgdt [GDT32_DESC]

    ; enable protected mode
    mov eax,cr0
    or eax,CR0_PE
    mov cr0,eax

    ; use long jump to reload $CS with selector which selects the code segment
    jmp dword CODE_SEG:protected_mode

protected_mode:
[bits 32]
    ; set other seg reg to the data segment
    mov ax,DATA_SEG
    mov ds,ax
    mov es,ax
    mov fs,ax
    mov gs,ax
    mov ss,ax

    ; zero 8KiB memory for 2-level page table, using 1GB page
    mov edi,PML4
    xor eax,eax
    mov ecx,0x800 
    cld
    rep stosd ;0x800 * 4 = 8KiB
    mov edi,PML4

    ; PML4E
    lea eax,[edi+0x1000]
    or eax,PAGE_PRESENT | PAGE_WRITE
    mov [edi],eax

    ; PDPTE, identity mapping of [0, 1GB], reused in map_kernel.
    xor eax,eax
    or eax,PAGE_PRESENT | PAGE_WRITE | PAGE_SIZE
    mov [edi+0x1000],eax

    ; enable PAE
    mov eax,cr4
    or eax,CR4_PAE
    mov cr4,eax

    ; load PML4 base to CR3
    mov eax,PML4
    mov cr3,eax

    ; set IA32_EFER.LME = 1
    mov ecx,EFER_REG
    rdmsr
    or eax,EFER_LME
    wrmsr

    ; enable paging
    mov eax,cr0
    or eax,CR0_PG
    mov cr0,eax

    ; load GDT64 for long mode
    lgdt [GDT64_DESC]

    jmp dword CODE_SEG:long_mode

long_mode:
[bits 64]
    ; set other seg reg to the data segment
    mov ax,DATA_SEG
    mov ds,ax
    mov es,ax
    mov fs,ax
    mov gs,ax
    mov ss,ax

    jmp mbr_c_entry


; Global Descriptor Table in protected mode, ref to https://wiki.osdev.org/Global_Descriptor_Table
align 8
GDT32_BASE: 
    ; gdt 0: null entry
    dd 0x00000000
    dd 0x00000000

    ; gdt 1: code seg
    dd 0x0000FFFF
    db 0x00,0x9A,0xCF,0x00

    ; gdt 2: data seg
    dd 0x0000FFFF
    db 0x00,0x92,0xCF,0x00

GDT32_DESC: ; GDT Descriptor will be loaded into GDTR
    dw GDT32_DESC-GDT32_BASE-1  ; first 2 bytes: GDT_LIMIT = sizeof(GDT) - 1
    dd GDT32_BASE               ; next 4 bytes: GDT32_BASE_ADDR


; Global Descriptor Table in long mode, 
; see Section 3.4.5: Segment Descriptors of the Intel Software Developer Manual, Volume 3-A.
; In 64-bit mode, the Base and Limit values are ignored, 
; each descriptor covers the entire linear address space regardless of what they are set to.
align 8
GDT64_BASE:
    ; gdt0: null entry
    dd 0x00000000
    dd 0x00000000            
 
    ; gdt1: code seg
    dd 0x00000000
    db 0x00,0x9A,0x20,0000

    ; gdt2: data seg
    dd 0x00000000
    db 0x00,0x92,0x00,0x00

GDT64_DESC: ; GDT Descriptor will be loaded into GDTR
    dw GDT64_DESC-GDT64_BASE-1  ; first 2 bytes: GDT_LIMIT = sizeof(GDT) - 1
    dq GDT64_BASE               ; next 4 bytes: GDT64_BASE_ADDR
