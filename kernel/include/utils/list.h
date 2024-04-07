#pragma once

#include "common/macro.h"
#include "common/type.h"

struct list_head {
  struct list_head* prev;
  struct list_head* next;
};

static inline void list_init(struct list_head* head) {
  head->next = head->prev = head;
}

static inline void list_add(struct list_head* head, struct list_head* new) {
  new->prev = head;
  new->next = head->next;
  head->next->prev = new;
  head->next = new;
}

static inline void list_del(struct list_head* node) {
  node->prev->next = node->next;
  node->next->prev = node->prev;
}

static inline bool is_empty_list(struct list_head* head) {
  return (head->next == head && head->prev == head);
}

static inline void init_list_head(struct list_head* head) {
  head->next = head;
  head->prev = head;
}

#define list_entry(type, ptr, field) container_of(type, ptr, field)

#define for_each_in_list(elem, type, field, head)      \
  for (elem = container_of((head)->next, type, field); \
       &((elem)->field) != (head);                     \
       elem = container_of(((elem)->field).next, type, field))

#define for_each_in_list_safe(elem, n, type, field, head)  \
  for (elem = container_of((head)->next, type, field),     \
      n = container_of(((elem)->field).next, type, field); \
       &((elem)->field) != (head);                         \
       elem = n, n = container_of(((n)->field).next, type, field))
