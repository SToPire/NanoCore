#pragma once

#include "common/type.h"
#include "layout.h"

#define PAGE_SIZE (4096)

struct page_hdr {
  paddr_t next;
};

void kfree(paddr_t addr);
paddr_t kalloc();
paddr_t kzalloc();

void mm_init();