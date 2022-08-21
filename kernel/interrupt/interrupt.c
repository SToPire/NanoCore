#include "interrupt/interrupt.h"
#include "common/cpu.h"
#include "common/klog.h"
#include "proc/process.h"
#include "syscall/syscall.h"

struct idt_gate_desc idt[IRQ_CNT];
extern void *allvectors[IRQ_CNT];

void exception_handler(struct trap_frame *tf) {
  switch (tf->trapno) {
  case IRQ_TIMER:
    if (get_cur_cpu()->cur_proc &&
        get_cur_cpu()->cur_proc->status == PROC_RUNNING)
      yield();
    lapic_eoi();
    break;
  case IRQ_SYSCALL:
    syscall_entry(tf);
    break;
  default: kerror("[exception handler] unknown irq type=%lu!\n", tf->trapno);
  }
}

static inline void set_idt_desc(int index, void *handler, u8 dpl) {
  struct idt_gate_desc *idt_desc = &idt[index];

  idt_desc->dpl = dpl;
  idt_desc->present = 1;
  idt_desc->ist = 0;
  idt_desc->type = IDTDESC_INTR_GATE;
  idt_desc->seg_selector = KCODE_SEG_SELECTOR;
  idt_desc->offset_high = (u64)handler >> 32;
  idt_desc->offset_mid = ((u64)handler >> 16) & 0xFFFF;
  idt_desc->offset_low = (u64)handler & 0xFFFF;
}

void idt_init() {
  for (int i = 0; i < IRQ_CNT; i++) {
    set_idt_desc(i, allvectors[i], 0);
  }
  set_idt_desc(IRQ_SYSCALL, allvectors[IRQ_SYSCALL], 3);

  lidt((u64)idt, sizeof(idt));
  sti();
}

void intr_init() {
  lapic_init();
  idt_init();
}