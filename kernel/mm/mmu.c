#include "mm/mmu.h"
#include "common/klog.h"
#include "common/macro.h"
#include "common/type.h"
#include "common/x86.h"
#include "mm/layout.h"
#include "mm/mm.h"
#include "utils/string.h"

extern char etext[];
extern char end[];

extern struct mem_pool global_mp;

extern int errno;

static struct init_mapping {
  vaddr_t virt;
  paddr_t p_beg;
  paddr_t p_end;
  u32 flag;
} init_mapping[] = {
    // kernel code
    {(vaddr_t)KERNEL_VSTART, (paddr_t)KERNEL_PSTART, V2P(etext), 0},

    // kernel data and physical memory
    {(vaddr_t)etext, V2P(etext), PHY_MAX_OFFSET, PTE_WRITE},
};

void __map_one_page(ptp_t *pgtbl, vaddr_t va, paddr_t pa, u32 flag) {
  ptp_t *ptp = pgtbl;

  for (int l = 4; l >= 1; l--) {
    u16 index;

    switch (l) {
    case 1: index = GET_L1_INDEX(va); break;
    case 2: index = GET_L2_INDEX(va); break;
    case 3: index = GET_L3_INDEX(va); break;
    case 4: index = GET_L4_INDEX(va); break;
    default: BUG("[__walkpgtbl] invalid level");
    }

    if (l == 1) {
      if (!ptp->ent[index].pte.is_valid) {
        ptp->ent[index].pte.is_valid = 1;
        ptp->ent[index].pte.is_user = 0;
        ptp->ent[index].pte.is_writeable = flag & PTE_WRITE;

        ptp->ent[index].pte.paddr = GET_PTE_ADDR(pa);
      }
    } else {
      if (!ptp->ent[index].pde.is_valid) {
        ptp->ent[index].pde.is_valid = 1;
        ptp->ent[index].pde.is_user = 0;
        ptp->ent[index].pde.is_writeable = 1;

        paddr_t tmp = kzalloc(&global_mp, PAGE_SIZE);
        if (!tmp) {
          kerror("kzalloc failed, errno=%d\n", errno);
          ABORT();
          return;
        }

        ptp->ent[index].pde.nxt_addr = GET_PTE_ADDR(tmp);
      }

      ptp = (ptp_t *)(ptp->ent[index].pde.nxt_addr << PAGE_SHIFT);
    }
  }
}

void init_kpgtbl() {
  int pgcnt = 0;

  ptp_t *pgtbl = (ptp_t *)kzalloc(&global_mp, PAGE_SIZE);
  if (!pgtbl) {
    kerror("kzalloc failed, errno=%d\n", errno);
    ABORT();
    return;
  }

  for (int i = 0; i < ARRSIZE(init_mapping); ++i) {
    paddr_t curp = ROUND_DOWN(init_mapping[i].p_beg, PAGE_SIZE);
    vaddr_t curv = ROUND_DOWN(init_mapping[i].virt, PAGE_SIZE);
    for (; curp < init_mapping[i].p_end; curp += PAGE_SIZE, curv += PAGE_SIZE) {
      __map_one_page(pgtbl, curv, curp, init_mapping[i].flag);
      ++pgcnt;
    }
  }

  kdebug("[init_kpgtbl] %d pages is mapped\n", pgcnt);

  lcr3((paddr_t)pgtbl);
}