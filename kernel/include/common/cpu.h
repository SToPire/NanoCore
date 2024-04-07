#pragma once

#include "common/x86.h"

#define NCPU (1)
#define NGDTENTRY (7)

#define KCODE_SEG_SELECTOR (1 << 3)
#define KDATA_SEG_SELECTOR (2 << 3)
#define UCODE_SEG_SELECTOR ((3 << 3) | 0x3)
#define UDATA_SEG_SELECTOR ((4 << 3) | 0x3)
#define TASK_SEG_SELECTOR (5 << 3)

// In 64-bit mode, the Base and Limit values are ignored.
struct gdt {
  u16 limit_lo;
  u16 base_lo;
  u8 base_mi;
  u8 access_byte;
  u8 flags;  // flags | limit[19:16]
  u8 base_hi;
};

struct trap_frame {
  u64 rax;
  u64 rbx;
  u64 rcx;
  u64 rdx;
  u64 rdi;
  u64 rsi;
  u64 rbp;
  u64 r8;
  u64 r9;
  u64 r10;
  u64 r11;
  u64 r12;
  u64 r13;
  u64 r14;
  u64 r15;

  u64 trapno;

  u64 errno;

  // below are defined by x86_64 hardware
  u64 rip;
  u64 cs;      // only 16 bits valid
  u64 eflags;  // only 32 bits valid
  u64 rsp;
  u64 ss;  // only 16 bits valid
};

/*
  We must prevent compiler from padding rsp0 to 8 bytes boudary!
*/
#pragma pack(push, 1)

struct task_state {
  u32 pad1;
  u64 rsp0;
  u64 rsp1;
  u64 rsp2;
  u64 pad2;
  u64 ist1;
  u64 ist2;
  u64 ist3;
  u64 ist4;
  u64 ist5;
  u64 ist6;
  u64 ist7;
  u64 pad3;
  u16 pad4;
  u16 iopb;
};

#pragma pack(pop)

// x86-64 callee saved registers and %rip
struct context {
  u64 rbx;
  u64 rbp;
  u64 r12;
  u64 r13;
  u64 r14;
  u64 r15;

  // return address is just above ctx
  u64 rip;
};

struct cpu {
  struct gdt gdtbl[NGDTENTRY];
  struct task_state ts;
  struct context* sched_ctx;
  struct process* cur_proc;
};

extern struct cpu cpu[NCPU];

static inline struct cpu* get_cur_cpu() {
  return &cpu[0];  // TODO: implement me!
}

void cpu_init();
