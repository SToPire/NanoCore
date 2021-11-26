/*
    Warning: Now MBR is 507 bytes in size, DO NOT ADD ANYTHING!!
 */
#include "include/ata.h"

void readsectn(void *addr, u16 sect, u16 cnt);

void mbr_c_entry() {
  readsect((void *)LOADER_MEMORY_LOCATION, LOADER_SECTOR);
  void (*loader_entry)(void);
  loader_entry = (void (*)(void))LOADER_MEMORY_LOCATION;
  loader_entry();
}
