#pragma once

#include "common/type.h"
#include "utils/list.h"

// forward declaration
struct slab_header;
struct mem_pool;

struct page {
  u8 order;
  bool alloc;
  struct list_head node;
  struct slab_header *slab;
} __attribute__((aligned(8)));

void *page_to_virt(struct mem_pool *mp, struct page *page);
struct page *virt_to_page(struct mem_pool *mp, void *addr);

void init_buddy(struct mem_pool *mp, vaddr_t start, vaddr_t end);
paddr_t buddy_alloc(struct mem_pool *mp, size_t size);
void buddy_free(struct mem_pool *mp, paddr_t addr);