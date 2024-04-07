#include "proc/schedule.h"
#include "common/cpu.h"
#include "common/klog.h"
#include "common/type.h"
#include "mm/mmu.h"
#include "proc/process.h"

extern struct process* proc_tbl[NPROC];
extern ptp_t* kpgtbl;

void context_switch(struct context** current, struct context* target);

void scheduler() {
  struct cpu* cpu = get_cur_cpu();
  struct process* proc;

  while (true) {
    for (int i = 0; i < NPROC; i++) {
      proc = proc_tbl[i];
      if (proc == NULL || proc->status != PROC_READY)
        continue;

      proc->status = PROC_RUNNING;
      cpu->cur_proc = proc;
      // kdebug("[scheduler] choose pid=%d to run.\n", proc->pid);
      switch_to_uproc(proc);

      context_switch(&cpu->sched_ctx, proc->ctx);

      // control transfer from user back to kernel
      lcr3((u64)kpgtbl);
    }
  }
}

void enter_schedule() {
  context_switch(&get_cur_proc()->ctx, cpu->sched_ctx);
}