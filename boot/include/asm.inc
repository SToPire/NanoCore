%define PML4        0x1000

%define PAGE_PRESENT    (1 << 0)
%define PAGE_WRITE      (1 << 1)
%define PAGE_SIZE       (1 << 7)

%define EFER_REG        0xC0000080
%define EFER_LME        (1 << 8)
%define EFER_NXE        (1 << 11)

%define CR0_PE          (1 << 0)
%define CR0_PG          (1 << 31)

%define CR4_PAE         (1 << 5)

%define CODE_SEG        (1 << 3)
%define DATA_SEG        (2 << 3)