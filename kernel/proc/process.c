#include "proc/process.h"
#include "common/cpu.h"
#include "common/errno.h"
#include "common/klog.h"
#include "common/type.h"
#include "common/x86.h"
#include "mm/layout.h"
#include "mm/mm.h"
#include "mm/mmu.h"

struct process *proc_tbl[NPROC];
void ret_to_user(void *sp); // defined in trampoline.S

void uproc_init() {
  vaddr_t kpage;
  paddr_t upage;
  struct process *proc;
  vaddr_t sp;

  kpage = P2V(kzalloc(PAGE_SIZE));
  proc = (struct process *)kpage;

  proc_tbl[0] = proc;
  proc->pid = 0;
  proc->parent = proc;
  proc->state = PROC_RUNNABLE;
  proc->canary = KSTACK_CANARY;

  sp = kpage + PAGE_SIZE;
  sp -= sizeof(struct trap_frame);
  proc->tf = (struct trap_frame *)sp;
  proc->tf->cs = UCODE_SEG_SELECTOR;
  proc->tf->ss = UDATA_SEG_SELECTOR;
  proc->tf->rip = 0x100000;             // fix me
  proc->tf->rsp = 0x100000 + PAGE_SIZE; // fix me
  proc->tf->eflags = EFLAG_IF;

  proc->pgtbl = (ptp_t *)kzalloc(PAGE_SIZE);
  set_kmapping(proc->pgtbl, false);
  upage = kzalloc(PAGE_SIZE);
  map_one_page(proc->pgtbl, 0x100000, upage, PTE_USER | PTE_WRITE, false);

  // set kstack
  mycpu()->ts.rsp0 = kpage + PAGE_SIZE;

  // TODO: remove me!
  *(u16 *)P2V(upage) = 0xfeeb;

  lcr3((paddr_t)proc->pgtbl);
  ret_to_user((void *)sp);
}