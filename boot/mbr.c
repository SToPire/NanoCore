#include "include/ata.h"
#include "include/layout.h"

void readsectn(void *addr, ushort sect, ushort cnt);

void mbr_c_entry() {
  readsectn((void *)LOADER_MEMORY_LOCATION, LOADER_SECTOR, 1);
  void (*loader_entry)(void);
  loader_entry = (void (*)(void))LOADER_MEMORY_LOCATION;
  loader_entry();
}
