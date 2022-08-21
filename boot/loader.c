/*
    Warning: Now loader is 502 bytes in size, remember to modify Makefile if ADD ANYTHING!!
 */
#include "include/ata.h"
#include "include/elf.h"

#define ROUND_UP(n, base) ((n + base - 1) / base)

void readsectn(void *addr, u32 sect, u8 cnt);
void map_kernel(); // defined in loader.S

void memset(void *addr, u8 val, u32 cnt) {
  u8 *p = (u8 *)addr;
  for (u32 i = 0; i < cnt; ++i)
    p[i] = val;
  return;
}

void loader_c_entry() {
  struct elf_hdr *elf;
  struct elf_program_header *ph, *eph;
  void (*kernel_entry)(void);

  readsectn((void *)ELFHDR_MEMORY_LOCATION, KERNEL_START_SECTOR, 1);
  elf = (struct elf_hdr *)ELFHDR_MEMORY_LOCATION;
  if (elf->e_magic != ELF_MAGIC) goto err;

  ph = (struct elf_program_header *)((void *)elf + elf->e_phoff);
  eph = ph + elf->e_phnum;
  for (; ph < eph; ++ph) {
    // addr must be aligned to sector boundary
    void *addr = (void *)ph->p_paddr - ph->p_offset % SECTOR_SIZE;
    u16 sect = ph->p_offset / SECTOR_SIZE + KERNEL_START_SECTOR;
    // take account of extra size due to alignment of addr
    u16 cnt = ROUND_UP(ph->p_filesz + ph->p_offset % SECTOR_SIZE, SECTOR_SIZE);
    readsectn(addr, sect, cnt);
    memset((void *)ph->p_paddr + ph->p_filesz, 0, ph->p_memsz - ph->p_filesz);
  }

  // page mapping: [VM_OFFSET, VM_OFFSET + 1GB] -> [0, 1GB]
  map_kernel();

  if (elf->e_entry < KERNEL_VSTART) goto err; // simple check
  kernel_entry = (void (*)(void))elf->e_entry;
  kernel_entry();

err:
  while (1)
    ;
}