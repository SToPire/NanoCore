#pragma once

#include "type.h"

static inline void outb(u16 port, u16 data) {
  asm volatile("out %0,%1" : : "a"(data), "d"(port));
}

static inline u8 inb(u16 port) {
  u8 data;
  asm volatile("in %1,%0" : "=a"(data) : "d"(port));
  return data;
}

static inline void
lcr3(u64 val)
{
  asm volatile("movq %0,%%cr3" : : "r" (val));
}