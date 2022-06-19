#include "mm/mm.h"
#include "common/macro.h"
#include "common/type.h"
#include "mm/layout.h"
#include "mm/mmu.h"
#include "utils/string.h"
#include "utils/util.h"

extern char end[];
static paddr_t kmem_freelist;

void kfree(paddr_t addr) {
  struct page_hdr *h = (struct page_hdr *)addr;
  h->next = kmem_freelist;
  kmem_freelist = addr;
}

paddr_t kalloc() {
  paddr_t ret = kmem_freelist;
  kmem_freelist = ((struct page_hdr *)kmem_freelist)->next;
  return ret;
}

paddr_t kzalloc() {
  paddr_t ret = kalloc();
  memset((void*)ret, 0, PAGE_SIZE);
  return ret;
}

void mm_init() {
  paddr_t page;
  BUG_ON((u64)end % PAGE_SIZE);

  /* 4MB (1024 pages) is reserved for kernel page table*/
  for (page = V2P(end); page < V2P(end) + PHASE1_PHYMEM_SIZE;
       page += PAGE_SIZE) {
    kfree(page);
  }

  init_kpgtbl();
}