#pragma once

#include "common/type.h"
#include "utils/list.h"

#define SLAB_INIT_SIZE          (256 * PAGE_SIZE)
#define SLAB_INIT_FALLBACK_SIZE (4 * PAGE_SIZE)

/* Each slab consists of several pages.
 * A list of slabs serves certain allocation size.
 */
struct slab_header {
  u8 order;

  u32 total_slot;
  u32 free_slot;
  vaddr_t slot_list_head;

  struct slab_header *next_slab;
} __attribute__((aligned(8)));

struct slot_list_node {
  vaddr_t next;
};

struct mem_pool;
void init_slab(struct mem_pool *mp);
paddr_t slab_alloc(struct mem_pool *mp, size_t size);
void slab_free(struct mem_pool *mp, paddr_t addr);