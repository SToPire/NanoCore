#include "mm/slab.h"
#include "common/klog.h"
#include "common/macro.h"
#include "common/type.h"
#include "mm/buddy.h"
#include "mm/layout.h"
#include "mm/mm.h"

extern struct mem_pool global_mp;
extern int errno;

vaddr_t _get_pages_for_slab(struct mem_pool* mp, size_t* ret_size) {
  vaddr_t ret_addr;

  *ret_size = SLAB_INIT_SIZE;
  ret_addr = P2V(buddy_alloc(mp, *ret_size));
  if (!ret_addr) {
    *ret_size = SLAB_INIT_FALLBACK_SIZE;
    ret_addr = P2V(buddy_alloc(mp, *ret_size));
    if (!ret_addr) {
      kerror("buddy_alloc failed, errno=%d", errno);
      ABORT();
    }
    WARN("Memory is scarce!");
  }

  return ret_addr;
}

struct slab_header* init_one_slab(struct mem_pool* mp, int order) {
  size_t buddy_alloc_size;
  void* pages;
  struct slab_header* slab_hdr;
  int slot_for_header;  // slots' count to store slab_hdr
  struct slot_list_node* slot_node;

  pages = (void*)_get_pages_for_slab(mp, &buddy_alloc_size);

  slot_for_header = sizeof(struct slab_header) / (1 << order);
  if (slot_for_header == 0)
    slot_for_header = 1;

  // Slab metadata stores in some slots.
  slab_hdr = (struct slab_header*)pages;
  slab_hdr->order = order;
  slab_hdr->total_slot = buddy_alloc_size / (1 << order) - slot_for_header;
  slab_hdr->free_slot = slab_hdr->total_slot;
  slab_hdr->next_slab = NULL;
  slab_hdr->slot_list_head = (vaddr_t)slab_hdr + slot_for_header * (1 << order);

  // Slots are organized as list.
  slot_node = (struct slot_list_node*)slab_hdr->slot_list_head;
  for (int i = 0; i < slab_hdr->total_slot - 1; i++) {
    slot_node->next = (vaddr_t)slot_node + (1 << order);
    slot_node = (struct slot_list_node*)slot_node->next;
  }
  slot_node->next = NULL;

  // each page in slab points to slab_hdr (used in slab_free())
  while (buddy_alloc_size) {
    virt_to_page(mp, pages)->slab = slab_hdr;
    pages += PAGE_SIZE;
    buddy_alloc_size -= PAGE_SIZE;
  }

  return slab_hdr;
}

void init_slab(struct mem_pool* mp) {
  for (int order = SLAB_MIN_ORDER; order <= SLAB_MAX_ORDER; order++) {
    mp->slabs[order - SLAB_MIN_ORDER] = init_one_slab(mp, order);
  }
}

paddr_t slab_alloc(struct mem_pool* mp, size_t size) {
  struct slab_header* slab;
  vaddr_t va;
  int order;

  order = SLAB_MIN_ORDER;
  while ((1 << order) < size)
    ++order;
  BUG_ON(order > SLAB_MAX_ORDER);

  slab = mp->slabs[order - SLAB_MIN_ORDER];
  BUG_ON(slab->order != order);
  if (likely(slab->free_slot != 0)) {
    va = slab->slot_list_head;
    slab->slot_list_head = ((struct slot_list_node*)va)->next;
    --slab->free_slot;
    return V2P(va);
  }

  slab->next_slab = init_one_slab(mp, order);
  slab = slab->next_slab;
  BUG_ON(slab->order != order);

  va = slab->slot_list_head;
  slab->slot_list_head = ((struct slot_list_node*)va)->next;
  --slab->free_slot;
  return V2P(va);
}

void slab_free(struct mem_pool* mp, paddr_t addr) {
  struct page* page;
  struct slab_header* slab;

  page = virt_to_page(mp, (void*)P2V(addr));
  slab = page->slab;

  ((struct slot_list_node*)P2V(addr))->next = slab->slot_list_head;
  slab->slot_list_head = P2V(addr);
  ++slab->free_slot;
}