#include "common/cpu.h"
#include "common/type.h"
#include "common/x86.h"

struct cpu cpu[NCPU];

void gdt_init(int cpuid) {
  u64 tss_base = (u64)&cpu[cpuid].ts;
  int tss_limit = sizeof(struct task_state) - 1;

  // empty
  cpu[cpuid].gdtbl[0] = (struct gdt){0, 0, 0, 0, 0, 0};
  // kcode
  cpu[cpuid].gdtbl[1] = (struct gdt){0, 0, 0, 0x9A, 0x20, 0};
  // kdata
  cpu[cpuid].gdtbl[2] = (struct gdt){0, 0, 0, 0x92, 0, 0};
  // ucode
  cpu[cpuid].gdtbl[3] = (struct gdt){0, 0, 0, 0xFA, 0x20, 0};
  // udata
  cpu[cpuid].gdtbl[4] = (struct gdt){0, 0, 0, 0xF2, 0, 0};

  // tss
  cpu[cpuid].gdtbl[5] = (struct gdt){tss_limit & 0xFFFF,
                                     tss_base & 0xFFFF,
                                     (tss_base >> 16) & 0xFF,
                                     0x89,
                                     0x40 | ((tss_limit >> 16) & 0xF),
                                     (tss_base >> 24) & 0xFF};

  // tss_hi
  *(u64 *)(&cpu[cpuid].gdtbl[6]) = (tss_base >> 32) & 0xFFFFFFFF;

  lgdt((u64)&cpu[cpuid].gdtbl, NGDTENTRY * sizeof(struct gdt));
}

void cpu_init() {
  gdt_init(0);
  ltr(TASK_SEG_SELECTOR);
}