#include "include/x86.h"

void readsectn(void *addr, ushort sect, ushort cnt);

void loader() {
  readsectn((void*)0x8000, 1, 1);
  while(1);
}

void readsectn(void *addr, ushort sect, ushort cnt) {
  while ((in(ATA_P_STATUS)&0xC0) != 0x40)
    ;

  out(ATA_P_SECTCNT, cnt);
  out(ATA_P_LBA_LO, sect & 0xFF);
  out(ATA_P_LBA_ME, (sect >> 8) & 0xFF);
  out(ATA_P_LBA_HI, (sect >> 16) & 0xFF);
  out(ATA_P_DEV, 0xE0 | ((sect >> 24) & 0xFF));
  out(ATA_P_CMD, ATA_READ_SECT_CMD);

  while ((in(ATA_P_STATUS)&0xC0) != 0x40)
    ;

  ushort *p = (ushort *)addr;
  for (int i = 0; i < cnt * SECTOR_SIZE / 2; i++) {
    p[i] = in(ATA_P_DATA);
  }
}