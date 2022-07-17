#pragma once

#include "common/type.h"

#define PAGE_ORDER (9)
#define PAGE_SHIFT (12)

#define L1_INDEX_SHIFT ((0 * PAGE_ORDER) + PAGE_SHIFT)
#define L2_INDEX_SHIFT ((1 * PAGE_ORDER) + PAGE_SHIFT)
#define L3_INDEX_SHIFT ((2 * PAGE_ORDER) + PAGE_SHIFT)
#define L4_INDEX_SHIFT ((3 * PAGE_ORDER) + PAGE_SHIFT)

#define GET_L1_INDEX(va)    ((va >> L1_INDEX_SHIFT) & ((1UL << PAGE_ORDER) - 1))
#define GET_L2_INDEX(va)    ((va >> L2_INDEX_SHIFT) & ((1UL << PAGE_ORDER) - 1))
#define GET_L3_INDEX(va)    ((va >> L3_INDEX_SHIFT) & ((1UL << PAGE_ORDER) - 1))
#define GET_L4_INDEX(va)    ((va >> L4_INDEX_SHIFT) & ((1UL << PAGE_ORDER) - 1))
#define GET_PAGE_OFFSET(va) (va & ((1UL << PAGE_SHIFT) - 1))

// length of next pte addr
#define PTE_ADDR_LEN       (40)
#define GET_PTE_ADDR(addr) ((addr >> PAGE_SHIFT) & ((1UL << PTE_ADDR_LEN) - 1))

#define PTE_WRITE   (1 << 0)
#define PTE_NONEXEC (1 << 1)

typedef union {
  /* L4, L3, L2 page table entry*/
  struct {
    u64 is_valid : 1, is_writeable : 1, is_user : 1, ign1 : 9;
    u64 nxt_addr : 40, ign2 : 11, non_execute : 1;
  } pde;

  /* L1 page table entry*/
  struct {
    u64 is_valid : 1, is_writeable : 1, is_user : 1, ign1 : 9;
    u64 paddr : 40, ign2 : 11, non_execute : 1;
  } pte;

  u64 val;
} pte_t;

/* page table page */
typedef struct {
  pte_t ent[1 << PAGE_ORDER];
} ptp_t;

void init_kpgtbl();