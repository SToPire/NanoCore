#define ushort unsigned short
#define uint   unsigned int

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

void out(ushort port, ushort data) {
  asm volatile("out %0,%1" : : "a"(data), "d"(port));
}

ushort in(ushort port) {
  ushort data;
  asm volatile("in %1,%0" : "=a"(data) : "d"(port));
  return data;
}