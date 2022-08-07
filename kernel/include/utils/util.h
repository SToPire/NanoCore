#pragma once

#include <common/type.h>

static inline void memset(void *dst, char ch, size_t size) {
  char *dst_ch = dst;

  for (size_t i = 0; i < size; i++) {
    dst_ch[i] = ch;
  }
}

static inline void memcpy(void *dst, void *src, size_t size) {
  char *dst_ch = dst, *src_ch = src;

  for (size_t i = 0; i < size; i++) {
    dst_ch[i] = src_ch[i];
  }
}