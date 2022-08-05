#include "proc/process.h"
#include "common/cpu.h"
#include "common/errno.h"
#include "common/klog.h"
#include "common/type.h"
#include "common/x86.h"
#include "mm/layout.h"
#include "mm/mm.h"
#include "mm/mmu.h"
#include "proc/schedule.h"
#include "utils/util.h"

struct process *proc_tbl[NPROC];
void iret_from_trapframe(); // defined in switch.S

void switch_to_uproc(struct process *proc) {
  // struct process itself is stored at bottom of kstack
  get_cur_cpu()->ts.rsp0 = (u64)proc + PAGE_SIZE;
  lcr3((paddr_t)proc->pgtbl);
}

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
  proc->status = PROC_RUNNABLE;
  proc->canary = KSTACK_CANARY;

  // set trapframe (for iret)
  sp = kpage + PAGE_SIZE;
  sp -= sizeof(struct trap_frame);
  proc->tf = (struct trap_frame *)sp;
  proc->tf->cs = UCODE_SEG_SELECTOR;
  proc->tf->ss = UDATA_SEG_SELECTOR;
  proc->tf->rip = USER_CODE_START;
  proc->tf->rsp = USER_CODE_START + PAGE_SIZE;
  proc->tf->eflags = EFLAG_IF;

  // set upgtbl
  proc->pgtbl = (ptp_t *)kzalloc(PAGE_SIZE);
  set_kmapping(proc->pgtbl, false);
  upage = kzalloc(PAGE_SIZE);
  map_one_page(proc->pgtbl, USER_CODE_START, upage, PTE_USER | PTE_WRITE,
               false);

  // TODO: remove me!
  *(u16 *)P2V(upage) = 0xfeeb;

  // set context
  sp -= sizeof(struct context);
  proc->ctx = (struct context *)sp;
  memset(proc->ctx, 0, sizeof(struct context));
  proc->ctx->rip = (u64)&iret_from_trapframe;
}

struct process *get_cur_proc() {
  // TODO: need pushcli when multiprocessor
  struct process *p;
  p = get_cur_cpu()->cur_proc;
  return p;
}

void yield() {
  struct process *proc = get_cur_proc();

  BUG_ON(proc->status != PROC_RUNNING);
  proc->status = PROC_RUNNABLE;
  enter_schedule();
}