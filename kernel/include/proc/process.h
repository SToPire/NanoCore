#pragma once

#include "common/cpu.h"
#include "common/type.h"
#include "common/x86.h"
#include "mm/mmu.h"
#include "vm.h"

#define NPROC (64)
#define KSTACK_CANARY (0x2333)

enum proc_stat {
  PROC_INVALID,
  PROC_RUNNING,
  PROC_READY,
  PROC_SLEEPING,
};

#define NOFILE 16

struct open_file {
  struct file* ofs[NOFILE];
  int of_count;
};

struct process {
  enum proc_stat status;
  struct process* parent;
  struct trap_frame* tf;
  struct mm_struct mm;
  struct open_file ofiles;
  struct context* ctx;
  ptp_t* pgtbl;
  u16 pid;
  u16 canary;
};

void switch_to_uproc(struct process* proc);
void uproc_init();

struct process* get_cur_proc();
void yield();
void exec(const char* path);