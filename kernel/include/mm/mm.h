#pragma once

#include "common/type.h"
#include "layout.h"
#include "utils/list.h"

#define PAGE_SIZE (4096)

// max phymem allocation size = 2 ^ (MAX_ORDER - 1) * PAGE_SIZE
#define BUDDY_MAX_ORDER 14

// slab's max/min allocation size = 2048/8 Bytes
#define SLAB_MAX_ORDER 11
#define SLAB_MIN_ORDER 3

// PHASE1_PHYMEM page header
struct page_hdr {
  paddr_t next;
};

struct free_list {
  u32 nr_free;
  struct list_head fl_head;
};

struct mem_pool {
  // buddy allocator
  struct free_list freelists[BUDDY_MAX_ORDER];
  // beginning of 'struct page' array
  vaddr_t page_meta_start;
  // beginning of page area
  vaddr_t page_area_start;
  // size of page area (in bytes)
  size_t size;

  // slab allocator
  struct slab_header *slabs[SLAB_MAX_ORDER - SLAB_MIN_ORDER + 1];

  // dummy allocator, list of free 4KB pages in PHASE1_PHYMEM area, see layout.h
  paddr_t phase1_page_list;
};

void kfree(paddr_t addr);
paddr_t kalloc(size_t size);
paddr_t kzalloc(size_t size);

void mm_init();