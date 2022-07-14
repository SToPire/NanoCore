#include "interrupt/interrupt.h"
#include "common/x86.h"

struct idt_gate_desc idt[IRQ_CNT];
extern void *allvectors[IRQ_CNT];

void exception_handler() {
  asm volatile("cli; hlt"); // dummy exception handler
}

static inline void set_idt_desc(int index, void *handler) {
  struct idt_gate_desc *idt_desc = &idt[index];

  idt_desc->dpl = 0;
  idt_desc->present = 1;
  idt_desc->ist = 0;
  idt_desc->type = IDTDESC_INTR_GATE;
  idt_desc->seg_selector = CODE_SEG_SELECTOR;
  idt_desc->offset_high = (u64)handler >> 32;
  idt_desc->offset_mid = ((u64)handler >> 16) & 0xFFFF;
  idt_desc->offset_low = (u64)handler & 0xFFFF;
}

void idt_init() {
  for (int i = 0; i < IRQ_CNT; i++) {
    set_idt_desc(i, allvectors[i]);
  }

  lidt((u64)idt, sizeof(idt));
  sti();
}