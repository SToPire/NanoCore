#pragma once

#include "common.h"

// HDD controller ports
#define ATA_P_DATA    0x1F0
#define ATA_P_FEATURE 0x1F1 // write to port
#define ATA_P_ERR     0x1F1 // read from port
#define ATA_P_SECTCNT 0x1F2
#define ATA_P_LBA_LO  0x1F3
#define ATA_P_LBA_ME  0x1F4
#define ATA_P_LBA_HI  0x1F5
#define ATA_P_DEV     0x1F6
#define ATA_P_CMD     0x1F7 // write to port
#define ATA_P_STATUS  0x1F7 // read from port

// HDD controller cmds
#define ATA_READ_SECT_CMD  0x20
#define ATA_WRITE_SECT_CMD 0x30

#define SECTOR_SIZE 512

void out(u16 port, u16 data) {
  asm volatile("out %0,%1" : : "a"(data), "d"(port));
}

u16 in(u16 port) {
  u16 data;
  asm volatile("in %1,%0" : "=a"(data) : "d"(port));
  return data;
}

// read sector indexed by `sect` from hdd into `addr`.
void readsect(void *addr, u32 sect) {
  while ((in(ATA_P_STATUS)&0xC0) != 0x40)
    ;

  // may be problematic when sectcnt is large, so read a sector each time to play safe. 
  out(ATA_P_SECTCNT, 0x1);

  out(ATA_P_LBA_LO, sect & 0xFF);
  out(ATA_P_LBA_ME, (sect >> 8) & 0xFF);
  out(ATA_P_LBA_HI, (sect >> 16) & 0xFF);
  out(ATA_P_DEV, 0xE0 | ((sect >> 24) & 0xF));
  out(ATA_P_CMD, ATA_READ_SECT_CMD);

  while ((in(ATA_P_STATUS)&0xC0) != 0x40)
    ;

  u16 *p = (u16 *)addr;
  for (int i = 0; i < SECTOR_SIZE / 2; i++) {
    p[i] = in(ATA_P_DATA);
  }
}

// read `cnt` sectors indexed by `sect` from hdd into `addr`.
void readsectn(void *addr, u32 sect, u8 cnt) {
  void *p = addr;
  for (u32 esect = sect + cnt; sect < esect; ++sect, p += SECTOR_SIZE)
    readsect(p, sect);
}