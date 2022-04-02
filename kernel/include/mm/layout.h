#pragma once

#define VM_OFFSET 0xFFFFFF0000000000
#define V2P(a)    ((paddr_t)((vaddr_t)a - VM_OFFSET))
#define P2V(a)    ((vaddr_t)((paddr_t)a + VM_OFFSET))

#define KERNEL_PSTART 0x100000
#define KERNEL_VSTART P2V(KERNEL_PSTART)

#define PHYMEM_SIZE    (512UL * 1024 * 1024)
#define PHY_MAX_OFFSET (KERNEL_PSTART + PHYMEM_SIZE)
