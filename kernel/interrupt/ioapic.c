/*
    This is the Intel 82093AA I/O APIC driver.
    Reference: https://wiki.osdev.org/IOAPIC
*/

#include "common/type.h"
#include "interrupt/interrupt.h"
#include "mm/layout.h"

// TODO: Detect this?
#define IOAPIC_BASE_ADDR P2V(0xFEC00000)

#define IOAPIC_IOREGSEL IOAPIC_BASE_ADDR
#define IOAPIC_IOWIN (IOAPIC_BASE_ADDR + 0x10)

#define IOAPIC_REG_ID 0x00
#define IOAPIC_REG_VER 0x01
#define IOAPIC_REG_ARB 0x02
#define IOAPIC_REG_RED_TBL_BASE 0x10

#define IOAPIC_RED_TBL_MASKED 0x10000

void ioapic_write(u32 reg, u32 data) {
  *(u32*)IOAPIC_IOREGSEL = reg;
  *(u32*)IOAPIC_IOWIN = data;
}

u32 ioapic_read(u32 reg) {
  *(u32*)IOAPIC_IOREGSEL = reg;
  return *(u32*)IOAPIC_IOWIN;
}

void ioapic_init() {
  int maxintr = (ioapic_read(IOAPIC_REG_VER) >> 16) & 0xFF;

  for (int i = 0; i < maxintr; i++) {
    ioapic_write(IOAPIC_REG_RED_TBL_BASE + 2 * i,
                 IOAPIC_RED_TBL_MASKED | (IRQ_DEV_BASE + i));
    ioapic_write(IOAPIC_REG_RED_TBL_BASE + 2 * i + 1, 0);
  }
}

void ioapic_enable(u8 irq, int cpu) {
  ioapic_write(IOAPIC_REG_RED_TBL_BASE + 2 * irq, IRQ_DEV_BASE + irq);
  ioapic_write(IOAPIC_REG_RED_TBL_BASE + 2 * irq + 1, cpu << 24);
}