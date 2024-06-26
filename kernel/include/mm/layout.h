#pragma once

#define VM_OFFSET 0xFFFFFF0000000000
#define V2P(a) ((paddr_t)((vaddr_t)a - VM_OFFSET))
#define P2V(a) ((vaddr_t)((paddr_t)a + VM_OFFSET))

#define KERNEL_PSTART 0x100000
#define KERNEL_VSTART P2V(KERNEL_PSTART)

#define PHYMEM_SIZE (512UL * 1024 * 1024)
#define PHY_MAX_OFFSET (KERNEL_PSTART + PHYMEM_SIZE)

#define PHASE1_PHYMEM_SIZE (4UL * 1024 * 1024)

#define USER_CODE_START 0x100000

#define USER_STACK_SIZE (0x1000 * 64)
#define USER_STACK_END VM_OFFSET
#define USER_STACK_START (USER_STACK_END - USER_STACK_SIZE)
