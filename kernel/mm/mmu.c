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
extern bool is_kpgtbl_set;
extern int errno;

ptp_t* kpgtbl;

static struct kernel_mapping {
  vaddr_t virt;
  paddr_t p_beg;
  paddr_t p_end;
  u32 flag;
} kernel_mapping[] = {
    // kernel code
    {(vaddr_t)KERNEL_VSTART, (paddr_t)KERNEL_PSTART, V2P(etext), 0},

    // kernel data and physical memory
    {(vaddr_t)etext, V2P(etext), PHY_MAX_OFFSET, PTE_WRITE | PTE_NONEXEC},

    // mapping 3.75-4 GB physical address to kernel for {l, io}apic MMIO
    {P2V(0xF0000000), 0xF0000000, 0x100000000, PTE_WRITE | PTE_NONEXEC},
};

void _traverse_pt(ptp_t* pgtbl, bool set_valid, vaddr_t va, paddr_t pa,
                  u32 flag, bool identity_mapping_on) {
  ptp_t* ptp = pgtbl;

  for (int l = 4; l >= 1; l--) {
    u16 index;

    // If identity page mapping is not enabled, we must use va.
    if (!identity_mapping_on)
      ptp = (ptp_t*)P2V(ptp);

    switch (l) {
      case 1:
        index = GET_L1_INDEX(va);
        break;
      case 2:
        index = GET_L2_INDEX(va);
        break;
      case 3:
        index = GET_L3_INDEX(va);
        break;
      case 4:
        index = GET_L4_INDEX(va);
        break;
      default:
        BUG("[__walkpgtbl] invalid level");
    }

    if (l == 1) {
      if (set_valid) {
        ptp->ent[index].pte.is_valid = 1;
        ptp->ent[index].pte.is_user = flag & PTE_USER ? 1 : 0;
        ptp->ent[index].pte.is_writeable = flag & PTE_WRITE ? 1 : 0;
        ptp->ent[index].pte.non_execute = flag & PTE_NONEXEC ? 1 : 0;
        ptp->ent[index].pte.paddr = GET_PTE_ADDR(pa);
      } else {
        ptp->ent[index].pte.is_valid = 0;
        ptp->ent[index].pte.is_user = 0;
        ptp->ent[index].pte.is_writeable = 0;
        ptp->ent[index].pte.non_execute = 0;
        ptp->ent[index].pte.paddr = 0;
      }
    } else {
      if (!ptp->ent[index].pde.is_valid) {
        if (set_valid) {
          ptp->ent[index].pde.is_valid = 1;
          /* According to Section 4.6.1, if the U/S flag (bit 2) is 0 in at least
           one of the paging-structure entries, the address is a supervisor-mode
           address. As a result, we must assign 1 as the default value for
           'is_user' field in page table structure in order to map user pages.
           */
          ptp->ent[index].pde.is_user = 1;
          ptp->ent[index].pde.is_writeable = 1;
          ptp->ent[index].pde.non_execute = 0;

          paddr_t tmp = kzalloc(PAGE_SIZE);

          ptp->ent[index].pde.nxt_addr = GET_PTE_ADDR(tmp);
        } else {
          return;
        }
      }

      ptp = (ptp_t*)((u64)(ptp->ent[index].pde.nxt_addr) << PAGE_SHIFT);
    }
  }
}

void map_one_page(ptp_t* pgtbl, vaddr_t va, paddr_t pa, u32 flag,
                  bool identity_mapping_on) {
  _traverse_pt(pgtbl, true, va, pa, flag, identity_mapping_on);
}

void unmap_one_page(ptp_t* pgtbl, vaddr_t va) {
  _traverse_pt(pgtbl, false, va, 0, 0, false);
}

int set_kmapping(ptp_t* pgtbl, bool identity_mapping_on) {
  int pgcnt = 0;
  for (int i = 0; i < ARRSIZE(kernel_mapping); ++i) {
    paddr_t curp = ROUND_DOWN(kernel_mapping[i].p_beg, PAGE_SIZE);
    vaddr_t curv = ROUND_DOWN(kernel_mapping[i].virt, PAGE_SIZE);
    for (; curp < kernel_mapping[i].p_end;
         curp += PAGE_SIZE, curv += PAGE_SIZE) {
      map_one_page(pgtbl, curv, curp, kernel_mapping[i].flag,
                   identity_mapping_on);
      ++pgcnt;
    }
  }
  return pgcnt;
}

void init_kpgtbl() {
  int pgcnt;

  kpgtbl = (ptp_t*)kzalloc(PAGE_SIZE);
  pgcnt = set_kmapping(kpgtbl, true);

  kdebug("[init_kpgtbl] %d pages is mapped\n", pgcnt);

  lcr3((paddr_t)kpgtbl);
  is_kpgtbl_set = true;
}