#pragma once

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

#define ELFHDR_MEMORY_LOCATION 0x500
#define LOADER_MEMORY_LOCATION 0x8000

#define LOADER_SECTOR       0x1
#define KERNEL_START_SECTOR 0x2

#define VM_OFFSET     0xFFFFFF0000000000
#define KERNEL_PSTART 0x100000
#define KERNEL_VSTART (KERNEL_PSTART + VM_OFFSET)
