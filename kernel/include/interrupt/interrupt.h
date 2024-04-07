#pragma once

#include "common/type.h"

#define IRQ_PF 0xE
#define IRQ_TIMER 0x40
#define IRQ_SYSCALL 0x80
#define IRQ_ERR (0xFF - 1)
#define IRQ_SPURIOUS 0xFF  // see Section 10.9, Vol 3A Part 1.

#define IRQ_CNT 256

// type field in struct idt_gate_desc
#define IDTDESC_INTR_GATE 0xE;
#define IDTDESC_TRAP_GATE 0xF;

struct idt_gate_desc {
  u16 offset_low;
  u16 seg_selector;

  u8 ist : 3;  // Interrupt Stack Table
  u8 rsv1 : 5;
  u8 type : 4;  // Interrupt/Trap gate
  u8 rsv2 : 1;
  u8 dpl : 2;
  u8 present : 1;
  u16 offset_mid;

  u32 offset_high;

  u32 rsv3;
};

void lapic_init();
void idt_init();

void intr_init();

// ack an interrupt
void lapic_eoi();
