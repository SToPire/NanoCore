#pragma once

#include "common/cpu.h"
#include "common/type.h"
#include "fs/vfs.h"
#include "utils/list.h"

#define VM_READ 0x1
#define VM_WRITE 0x2
#define VM_EXEC 0x4
#define VM_USER 0x8

struct vm_area_struct {
  u64 start;
  u64 end;

  u64 flags;

  struct file* file;
  u64 file_offset;
  u64 file_size;

  struct list_head list;
};

struct mm_struct {
  struct list_head mmap_list;
};

struct pf_error {
  union {
    struct {
      u32 present : 1;
      u32 write : 1;
      u32 user : 1;
      u32 rsvd : 1;
      u32 fetch : 1;
      u32 pkey : 1;
      u32 ss : 1;
      u32 hlat : 1;
      u32 padding1 : 7;
      u32 sgx : 1;
      u32 padding2 : 16;
    };

    u32 val;
  };
};

void init_mm_struct(struct mm_struct* mm);
void add_vma(struct mm_struct* mm, vaddr_t start, vaddr_t end, u64 flags,
             struct file* file, u64 file_offset, u64 file_size);

void pagefault_handler(struct trap_frame* tf);