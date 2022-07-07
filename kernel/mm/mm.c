#include "mm/mm.h"
#include "common/klog.h"
#include "common/macro.h"
#include "common/type.h"
#include "mm/buddy.h"
#include "mm/layout.h"
#include "mm/mmu.h"
#include "utils/string.h"
#include "utils/util.h"

extern char end[];
struct mem_pool global_mp;

void kfree(struct mem_pool *mp, paddr_t addr) {
  // PHASE1_PHYMEM area
  if (addr < V2P(end) + PHASE1_PHYMEM_SIZE) {
    struct page_hdr *h = (struct page_hdr *)addr;
    h->next = mp->phase1_page_list;
    mp->phase1_page_list = addr;
    return;
  }

  return buddy_free(mp, addr);
}

paddr_t kalloc(struct mem_pool *mp, size_t size) {
  if (size == PAGE_SIZE && global_mp.phase1_page_list) {
    paddr_t ret = mp->phase1_page_list;
    mp->phase1_page_list = ((struct page_hdr *)mp->phase1_page_list)->next;
    return ret;
  }

  return buddy_alloc(mp, size);
}

paddr_t kzalloc(struct mem_pool *mp, size_t size) {
  paddr_t ret = kalloc(mp, size);
  if (!ret) {
    return (paddr_t)NULL;
  }

  memset((void *)ret, 0, PAGE_SIZE);
  return ret;
}

void init_mempool(struct mem_pool *mp) {
  // init mem_pool
  for (int i = 0; i < MAX_ORDER; i++) {
    mp->freelists[i].nr_free = 0;
    list_init(&mp->freelists[i].fl_head);
  }
  mp->phase1_page_list = (paddr_t)NULL;
}

/*  PHASE1_PHYMEM is fix-sized area consist of 4KB pages.
 *  It's mainly used for setting up kernel page table
 *  when buddy allocator is not available.
 */
void init_phase1_phymem(struct mem_pool *mp) {
  paddr_t page;
  /* 4MB (1024 pages) is reserved for kernel page table*/
  for (page = V2P(end); page < V2P(end) + PHASE1_PHYMEM_SIZE;
       page += PAGE_SIZE) {
    kfree(&global_mp, page);
  }
}

void mm_init() {
  BUG_ON((u64)end % PAGE_SIZE);

  init_mempool(&global_mp);

  init_phase1_phymem(&global_mp);
  init_kpgtbl();

  init_buddy(&global_mp, (vaddr_t)(end + PHASE1_PHYMEM_SIZE), P2V(PHYMEM_SIZE));
}