#include "common/klog.h"
#include "common/macro.h"
#include "common/type.h"
#include "common/x86.h"
#include "interrupt/interrupt.h"
#include "mm/layout.h"

// see https://wiki.osdev.org/APIC

// Chapter 10 of Intel System Programming Guide, Vol 3A Part 1.
#define LAPIC_BASE_MSR 0x1B
#define LAPIC_BASE_MASK (~0xFFF)

#define LAPIC_TIMER_REG 0x320
// Initlal/Current Count Register (for Timer)
#define LAPIC_TIMER_TICR 0x380
#define LAPIC_TIMER_TCCR 0x390
// Divide Configuration Register (for Timer)
#define LAPIC_TIMER_DCR 0x3E0
#define TIMER_PERIODIC_BIT (1 << 17)

// Spurious Interrupt Vector Register
#define LAPIC_SVT_REG 0xF0
#define SVT_ENABLE (1 << 8)

// Interrupt Command Register (Section 10.6)
#define LAPIC_ICR_LO 0x300
#define LAPIC_ICR_HI 0x310
#define ICR_DEST_ALL_INCLUDE_SELF (1 << 19)
#define ICR_DELIVMODE_INIT ((1 << 10) | (1 << 8))
#define ICR_TRIGMODE_LEVEL (1 << 15)
#define ICR_DELIV_STAT (1 << 12)
// sync all local APICs' arbitration IDs
#define ICR_SYNC_ARBID \
  (ICR_DEST_ALL_INCLUDE_SELF | ICR_DELIVMODE_INIT | ICR_TRIGMODE_LEVEL)

// Logical Interrupt Register
#define LAPIC_LINT0_REG 0x350
#define LAPIC_LINT1_REG 0x360
// Thermal Monitor Register
#define LAPIC_THM_REG 0x330
// Performance Counter Register
#define LAPIC_PC_REG 0x340
// Error Register
#define LAPIC_ERR_REG 0x370

// End-of-interrupt Register
#define LAPIC_EOI_REG 0xB0

// Task Priority Register
#define LAPIC_TP_REG 0x80

// masked interrupt will not be sent to CPU
#define LAPIC_LVT_MASKED (1 << 16)

vaddr_t lapic_base;

static inline vaddr_t get_lapic_base_addr() {
  return P2V(rdmsr(LAPIC_BASE_MSR)) & LAPIC_BASE_MASK;
}

static inline void lapic_write(vaddr_t base, u64 off, u64 size, u64 cont) {
  switch (size) {
    case 1:
      *(u8*)((void*)base + off) = (u8)cont;
      break;
    case 2:
      *(u16*)((void*)base + off) = (u16)cont;
      break;
    case 4:
      *(u32*)((void*)base + off) = (u32)cont;
      break;
    case 8:
      *(u64*)((void*)base + off) = (u64)cont;
      break;
    default:
      kerror("[lapic_write] Invalid param!\n");
      ABORT();
  }
  cont = *(u8*)((void*)base + off);
}

static inline u64 lapic_read(vaddr_t base, u64 off, u64 size) {
  switch (size) {
    case 1:
      return *(u8*)((void*)base + off);
    case 2:
      return *(u16*)((void*)base + off);
    case 4:
      return *(u32*)((void*)base + off);
    case 8:
      return *(u64*)((void*)base + off);
    default:
      kerror("[lapic_read] Invalid param!\n");
      ABORT();
  }
}

void lapic_init() {
  lapic_base = get_lapic_base_addr();

  // SVT (Section 10.9)
  lapic_write(lapic_base, LAPIC_SVT_REG, 4, SVT_ENABLE | IRQ_SPURIOUS);

  // enable timer (Section 10.5.4)
  lapic_write(lapic_base, LAPIC_TIMER_REG, 4, TIMER_PERIODIC_BIT | IRQ_TIMER);
  lapic_write(lapic_base, LAPIC_TIMER_DCR, 4, 0);
  lapic_write(lapic_base, LAPIC_TIMER_TICR, 4, 1000 * 1000 * 10);

  // Local interrupt pins are not needed in MP System, see Section 10.1
  lapic_write(lapic_base, LAPIC_LINT0_REG, 4, LAPIC_LVT_MASKED);
  lapic_write(lapic_base, LAPIC_LINT1_REG, 4, LAPIC_LVT_MASKED);

  // We DO NOT need Thermal Monitor & Performance Counter
  lapic_write(lapic_base, LAPIC_THM_REG, 4, LAPIC_LVT_MASKED);
  lapic_write(lapic_base, LAPIC_PC_REG, 4, LAPIC_LVT_MASKED);
  // but need to detect internal APIC error
  lapic_write(lapic_base, LAPIC_ERR_REG, 4, ~LAPIC_LVT_MASKED & IRQ_ERR);

  // Ack any on-going interrupts
  lapic_write(lapic_base, LAPIC_EOI_REG, 4, 0);

  // send 'INIT Level De-assert' to sync LAPIC ArbIDs, see Section 10.6.1
  lapic_write(lapic_base, LAPIC_ICR_HI, 4, 0);
  lapic_write(lapic_base, LAPIC_ICR_LO, 4, ICR_SYNC_ARBID);
  while (lapic_read(lapic_base, LAPIC_ICR_LO, 4) & ICR_DELIV_STAT)
    ;

  // Do not block any interrupts, see Section 10.8.3.1
  lapic_write(lapic_base, LAPIC_TP_REG, 4, 0);
}

void lapic_eoi() {
  lapic_write(lapic_base, LAPIC_EOI_REG, 4, 0);
}
