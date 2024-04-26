#pragma once

#include "common/type.h"
#include "utils/list.h"

// forward declaration
struct mem_pool;

void init_buddy(struct mem_pool* mp, vaddr_t start, vaddr_t end);
paddr_t buddy_alloc(struct mem_pool* mp, size_t size);
void buddy_free(struct mem_pool* mp, paddr_t addr);