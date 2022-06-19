#pragma once

#include <common/type.h>

static inline void memset(void *dst, char ch, size_t size) {
  char *dst_ch = dst;

  for (size_t i = 0; i < size; i++) {
    dst_ch[i] = ch;
  }
}