#pragma once

#include "common/type.h"
#include "utils/list.h"

// max phymem allocation size = 2 ^ (MAX_ORDER - 1) * PAGE_SIZE
#define MAX_ORDER 14

struct free_list {
  u32 nr_free;
  struct list_head fl_head;
};

struct mem_pool {
  struct free_list freelists[MAX_ORDER];

  // beginning of 'struct page' array
  vaddr_t page_meta_start;
  // beginning of page area
  vaddr_t page_area_start;
  // size of page area (in bytes)
  size_t size;

  // list of free 4KB pages in PHASE1_PHYMEM area, see layout.h
  paddr_t phase1_page_list;
};

struct page {
  u8 order;
  bool alloc;
  struct list_head node;
} __attribute__((aligned(8)));

void init_buddy(struct mem_pool *mp, vaddr_t start, vaddr_t end);
paddr_t buddy_alloc(struct mem_pool *mp, size_t size);
void buddy_free(struct mem_pool *mp, paddr_t addr);