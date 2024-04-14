#pragma once

#include "common/klog.h"
#include "common/type.h"
#include "common/x86.h"

#define ATA_DATA 0x1F0
#define ATA_FEATURE 0x1F1  // write only
#define ATA_ERR 0x1F1      // read only
#define ATA_SECTCNT 0x1F2
#define ATA_LBA_LO 0x1F3
#define ATA_LBA_ME 0x1F4
#define ATA_LBA_HI 0x1F5
#define ATA_DEV 0x1F6
#define ATA_CMD 0x1F7     // write only
#define ATA_STATUS 0x1F7  // read only

#define ATA_READ_CMD 0x20
#define ATA_WRITE_CMD 0x30
#define ATA_READ_MULTI 0xC4
#define ATA_WRITE_MULTI 0xC5

#define ATA_SECTOR_SIZE 512

static void ata_readsectn(void* addr, bool master, u32 sect, u8 cnt) {
  // do not support cnt > 16
  if (cnt > 16) {
    kerror("ata_readsectn: cnt > 16\n");
    return;
  }

  while ((inb(ATA_STATUS) & 0xC0) != 0x40)
    ;

  u8 read_cmd = (cnt == 1) ? ATA_READ_CMD : ATA_READ_MULTI;
  u8 dev_val = master ? 0xE0 : 0xF0;

  outb(ATA_SECTCNT, cnt);

  outb(ATA_LBA_LO, sect & 0xFF);
  outb(ATA_LBA_ME, (sect >> 8) & 0xFF);
  outb(ATA_LBA_HI, (sect >> 16) & 0xFF);
  outb(ATA_DEV, dev_val | ((sect >> 24) & 0xF));
  outb(ATA_CMD, read_cmd);

  // TODO: use interrupt
  while ((inb(ATA_STATUS) & 0xC0) != 0x40)
    ;

  u16* p = (u16*)addr;
  for (int i = 0; i < ATA_SECTOR_SIZE * (int)cnt / 2; i++) {
    p[i] = inw(ATA_DATA);
  }
}
