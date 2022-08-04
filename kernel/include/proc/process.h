#pragma once

#include "common/type.h"
#include "common/x86.h"
#include "mm/mmu.h"

#define NPROC         (64)
#define KSTACK_CANARY (0x2333)

enum proc_stat {
  PROC_UNUSED,
  PROC_RUNNING,
  PROC_RUNNABLE,
  PROC_SLEEPING,
};

struct process {
  enum proc_stat state;
  u16 pid;
  struct process *parent;
  struct trap_frame *tf;
  ptp_t* pgtbl;
  u16 canary;
};

void uproc_init();