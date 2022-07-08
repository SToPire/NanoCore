#include "mm/buddy.h"
#include "common/errno.h"
#include "common/klog.h"
#include "common/macro.h"
#include "common/type.h"
#include "mm/mm.h"
#include "mm/mmu.h"
#include "utils/list.h"
#include "utils/util.h"

extern int errno;

void _print_buddy_info(struct mem_pool *mp) {
  u64 sum = 0;
  for (int i = BUDDY_MAX_ORDER - 1; i >= 0; i--) {
    u8 nr_free = mp->freelists[i].nr_free;
    if (nr_free) {
      sum += (1 << i) * nr_free;
      kdebug("[buddy_init]    freelist[%d].nr_free=%d\n", i, nr_free);
    }
  }
  kdebug("[buddy_init] total=%d\n", sum);
}

void init_buddy(struct mem_pool *mp, vaddr_t start, vaddr_t end) {
  vaddr_t real_start, real_end;
  struct page *pgmeta;
  int pgcnt;
  int step = 1, order = 0;

  real_start = ROUND_UP(start, PAGE_SIZE);
  real_end = ROUND_DOWN(end, PAGE_SIZE);
  pgcnt = (real_end - real_start) / (PAGE_SIZE + sizeof(struct page));

  pgmeta = (struct page *)real_start;
  mp->page_meta_start = (vaddr_t)pgmeta;

  // pages_start should be ROUND_UP to 4KB, hence (potentially) lose 1 page
  mp->page_area_start = (vaddr_t)pgmeta + pgcnt * sizeof(struct page);
  if (mp->page_area_start % PAGE_SIZE != 0) {
    mp->page_area_start = ROUND_UP(mp->page_area_start, PAGE_SIZE);
    pgcnt--;
  }
  mp->size = real_end - mp->page_area_start;

  // init pgmeta
  memset(pgmeta, 0, sizeof(struct page) * pgcnt);
  for (int i = 0; i < pgcnt; i++) {
    list_init(&pgmeta[i].node);
  }

  while ((step << 1) <= pgcnt && order < BUDDY_MAX_ORDER - 1) {
    step <<= 1;
    order++;
  }

  kdebug("[buddy_init] pgcnt=%d with BUDDY_MAX_ORDER=%d\n", pgcnt, order);

  for (int remain = pgcnt; remain;) {
    int pgnum = pgcnt - remain;
    pgmeta[pgnum].order = order;

    mp->freelists[order].nr_free++;
    list_add(&mp->freelists[order].fl_head, &pgmeta[pgnum].node);

    remain -= step;
    while (remain < step) {
      step >>= 1;
      order--;
    }
  }

  _print_buddy_info(mp);
}

/* convert between "struct page*" and page pointer*/
void *page_to_virt(struct mem_pool *mp, struct page *page) {
  u64 pgnum = page - (struct page *)mp->page_meta_start;
  return (void *)mp->page_area_start + pgnum * PAGE_SIZE;
}

struct page *virt_to_page(struct mem_pool *mp, void *addr) {
  u64 pgnum = ((vaddr_t)addr - mp->page_area_start) / PAGE_SIZE;
  return (struct page *)mp->page_meta_start + pgnum;
}

struct page *_get_buddy(struct mem_pool *mp, struct page *free_blk, int order) {
  void *blk_addr, *buddy_addr;
  u64 offset;

  blk_addr = page_to_virt(mp, free_blk);
  offset = (u64)(blk_addr - mp->page_area_start);

  offset ^= (1ULL << (PAGE_SHIFT + order));
  buddy_addr = (void *)mp->page_area_start + offset;

  BUG_ON((vaddr_t)buddy_addr < mp->page_area_start ||
         (vaddr_t)buddy_addr > mp->page_area_start + mp->size);
  return virt_to_page(mp, buddy_addr);
}

void _split_blk(struct mem_pool *mp, struct page *free_blk, int cur_order,
                int target_order) {
  BUG_ON(cur_order != free_blk->order);
  if (cur_order == target_order) return;

  struct page *buddy_blk = _get_buddy(mp, free_blk, cur_order - 1);

  free_blk->order = buddy_blk->order = cur_order - 1;

  // add buddy block to freelist
  list_add(&mp->freelists[cur_order - 1].fl_head, &buddy_blk->node);
  ++mp->freelists[cur_order - 1].nr_free;

  _split_blk(mp, free_blk, cur_order - 1, target_order);
}

/*
  recursively merge block, return merged block's pagemeta via 'blk'
*/
void _merge_blk(struct mem_pool *mp, struct page **blk) {
  struct page *buddy, *tmp;
  int order;

  order = (*blk)->order;
  if (order == BUDDY_MAX_ORDER - 1) return;

  buddy = _get_buddy(mp, *blk, order);
  if (order != buddy->order || buddy->alloc) return;

  list_del(&buddy->node);
  --mp->freelists[buddy->order].nr_free;

  if (buddy < *blk) {
    tmp = buddy;
    buddy = *blk;
    *blk = tmp;
  }

  memset(buddy, 0, sizeof(struct page));
  (*blk)->order = order + 1;
  (*blk)->alloc = false;

  _merge_blk(mp, blk);
}

paddr_t buddy_alloc(struct mem_pool *mp, size_t size) {
  if(size==4096) kdebug("here");
  int order = 0;

  while ((1UL << order) * PAGE_SIZE < size)
    order++;

  if (order >= BUDDY_MAX_ORDER) {
    errno = -EINVAL;
    return (paddr_t)NULL;
  }

  for (int cur_order = order; cur_order < BUDDY_MAX_ORDER; ++cur_order) {
    if (mp->freelists[cur_order].nr_free > 0) {
      struct list_head *free_node = mp->freelists[cur_order].fl_head.next;
      struct page *free_blk = list_entry(free_node, struct page, node);

      list_del(free_node);
      --mp->freelists[cur_order].nr_free;

      _split_blk(mp, free_blk, cur_order, order);
      BUG_ON(free_blk->order != order);

      free_blk->alloc = true; 
      return V2P(page_to_virt(mp, free_blk));
    }
  }

  errno = -ENOMEM;
  return (paddr_t)NULL;
}

void buddy_free(struct mem_pool *mp, paddr_t addr) {
  struct page *blk;

  BUG_ON(P2V(addr) < mp->page_area_start ||
         P2V(addr) > mp->page_area_start + mp->size);

  blk = virt_to_page(mp, (void *)P2V(addr));
  blk->alloc = false;

  _merge_blk(mp, &blk);

  list_add(&mp->freelists[blk->order].fl_head, &blk->node);
  ++mp->freelists[blk->order].nr_free;
}