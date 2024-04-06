#include "proc/vm.h"
#include "common/klog.h"
#include "common/macro.h"
#include "common/type.h"
#include "common/x86.h"
#include "mm/mm.h"
#include "mm/mmu.h"
#include "proc/process.h"

void init_mm_struct(struct mm_struct *mm) { init_list_head(&mm->mmap_list); }

void add_vma(struct mm_struct *mm, vaddr_t start, vaddr_t end, u64 flags) {
  struct vm_area_struct *vma, *n;
  struct list_head *prev = &mm->mmap_list;

  for_each_in_list_safe(vma, n, struct vm_area_struct, list, &mm->mmap_list) {
    if (vma->end <= start) {
      prev = &vma->list;
      continue;
    }
    if (vma->start >= end) break;

    if (start <= vma->start && end >= vma->end) {
      // new area covers the old one
      list_del(&vma->list);
      kfree(V2P(vma));
      continue;
    }

    if (start > vma->start && end < vma->end) {
      // new area is inside the old one
      struct vm_area_struct *mid_vma =
          (struct vm_area_struct *)P2V(kalloc(sizeof(struct vm_area_struct)));
      struct vm_area_struct *end_vma =
          (struct vm_area_struct *)P2V(kalloc(sizeof(struct vm_area_struct)));

      mid_vma->start = start;
      mid_vma->end = end;
      mid_vma->flags = flags;
      list_add(&vma->list, &mid_vma->list);

      end_vma->start = end;
      end_vma->end = vma->end;
      end_vma->flags = vma->flags;
      list_add(&mid_vma->list, &end_vma->list);

      vma->end = start;
      return;
    }

    if (start <= vma->start) {
      // new area is at the beginning of the old one
      vma->start = end;
    } else if (end >= vma->end) {
      // new area is at the end of the old one
      vma->end = start;
      prev = &vma->list;
    }
  }

  // contiguous with the previous one
  vma = container_of(prev, struct vm_area_struct, list);
  if (vma->end == start && vma->flags == flags) {
    vma->end = end;
    return;
  }

  vma = (struct vm_area_struct *)P2V(kalloc(sizeof(struct vm_area_struct)));
  vma->start = start;
  vma->end = end;
  vma->flags = flags;
  list_add(prev, &vma->list);
}

struct vm_area_struct *find_vma(struct mm_struct *mm, vaddr_t addr) {
  struct vm_area_struct *vma;

  for_each_in_list(vma, struct vm_area_struct, list, &mm->mmap_list) {
    if (addr >= vma->start && addr < vma->end) return vma;
  }

  return NULL;
}

u32 vma_flags_to_pte_flags(u64 flags) {
  u32 pte_flags = 0;

  if (flags & VM_WRITE) pte_flags |= PTE_WRITE;
  if (!(flags & VM_EXEC)) pte_flags |= PTE_NONEXEC;
  if (flags & VM_USER) pte_flags |= PTE_USER;

  return pte_flags;
}

void pagefault_handler(struct trap_frame *tf) {
  struct vm_area_struct *vma;
  struct pf_error *err;
  struct process *proc = get_cur_proc();
  vaddr_t va;
  paddr_t pa;
  u32 flags;

  if (proc == NULL) {
    kerror("page fault: no current process\n");
    BUG_ON(true);
  }

  va = rcr2();
  err = (struct pf_error *)&tf->errno;

  vma = find_vma(&proc->mm, va);
  if (!vma) {
    kerror("page fault: no vma found for va=%lx\n", va);
    BUG_ON(true);
  }

  kdebug("[pf handler] err=%lx, va=%lx\n", err->val, va);
  if (!err->present) {
    if (err->write && !(vma->flags & VM_WRITE)) {
      kerror("page fault: write to read-only page for va=%lx\n", va);
      BUG_ON(true);
    }

    if (err->user && !(vma->flags & VM_USER)) {
      kerror("page fault: user access to kernel page for va=%lx\n", va);
      BUG_ON(true);
    }

    if (err->fetch && !(vma->flags & VM_EXEC)) {
      kerror("page fault: fetch from non-executable page for va=%lx\n", va);
      BUG_ON(true);
    }

    pa = kzalloc(PAGE_SIZE);
    flags = vma_flags_to_pte_flags(vma->flags);

    map_one_page(proc->pgtbl, va, pa, flags, false);
    // lcr3((paddr_t)proc->pgtbl);
  }

  return;
}