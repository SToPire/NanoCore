#pragma once

#include "common/type.h"
#include "layout.h"
#include "mm/buddy.h"

#define PAGE_SIZE (4096)

struct page_hdr {
  paddr_t next;
};

void kfree(struct mem_pool *mp, paddr_t addr);
paddr_t kalloc(struct mem_pool *mp, size_t size);
paddr_t kzalloc(struct mem_pool *mp, size_t size);

void mm_init();