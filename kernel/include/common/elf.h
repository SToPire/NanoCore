#pragma once

#include "common/type.h"

#define ELF_MAGIC 0x464C457FU

#define PT_NULL 0
#define PT_LOAD 1

#define PF_X 1
#define PF_W 2
#define PF_R 4

struct elf_header {
  u32 e_magic;
  u8 e_indent[12];
  u16 e_type;
  u16 e_machine;
  u32 e_version;
  u64 e_entry;
  u64 e_phoff;
  u64 e_shoff;
  u32 e_flags;
  u16 e_ehsize;
  u16 e_phentsize; /* The size of a program header table entry */
  u16 e_phnum;     /* The number of entries in the program header table */
  u16 e_shentsize; /* The size of a section header table entry */
  u16 e_shnum;     /* The number of entries in the section header table */
  u16 e_shstrndx;  /* Index of the section header table entry that
                      contains the section names. */
};

struct elf_program_header {
  u32 p_type;
  u32 p_flags;
  u64 p_offset;
  u64 p_vaddr;
  u64 p_paddr;
  u64 p_filesz;
  u64 p_memsz;
  u64 p_align;
};