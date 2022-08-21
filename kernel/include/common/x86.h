#pragma once

#include "type.h"

#define EFLAG_IF (1 << 9)

static inline void outb(u16 port, u8 data) {
  asm volatile("out %0,%1" : : "a"(data), "d"(port));
}

static inline void outw(u16 port, u16 data) {
  asm volatile("out %0,%1" : : "a"(data), "d"(port));
}

static inline u8 inb(u16 port) {
  u8 data;
  asm volatile("in %1,%0" : "=a"(data) : "d"(port));
  return data;
}

static inline u16 inw(u16 port) {
  u16 data;
  asm volatile("in %1,%0" : "=a"(data) : "d"(port));
  return data;
}

static inline void lcr3(u64 val) { asm volatile("movq %0,%%cr3" : : "r"(val)); }

static inline void lidt(u64 p, int size) {
  volatile u16 pd[5];

  pd[0] = size - 1;
  pd[1] = p;
  pd[2] = p >> 16;
  pd[3] = p >> 32;
  pd[4] = p >> 48;

  asm volatile("lidt (%0)" : : "r"(pd));
}

static inline void lgdt(u64 p, int size) {
  volatile u16 pd[5];

  pd[0] = size - 1;
  pd[1] = p;
  pd[2] = p >> 16;
  pd[3] = p >> 32;
  pd[4] = p >> 48;

  asm volatile("lgdt (%0)" : : "r"(pd));
}

static inline void ltr(u16 sel) { asm volatile("ltr %0" : : "r"(sel)); }

// see https://wiki.osdev.org/Inline_Assembly/Examples
static inline u64 rdmsr(u64 msr) {
  u32 low, high;
  asm volatile("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
  return ((u64)high << 32) | low;
}

static inline void wrmsr(u64 msr, u64 value) {
  u32 low = value & 0xFFFFFFFF;
  u32 high = value >> 32;
  asm volatile("wrmsr" : : "c"(msr), "a"(low), "d"(high));
}

static inline void cli(void) { asm volatile("cli"); }

static inline void sti(void) { asm volatile("sti"); }