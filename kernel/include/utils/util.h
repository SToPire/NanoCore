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

static inline int memcmp(const void *str1, const void *str2, size_t n) {
  const char *str1_ch = str1, *str2_ch = str2;

  for (size_t i = 0; i < n; i++) {
    if (str1_ch[i] < str2_ch[i]) return -1;
    if (str1_ch[i] > str2_ch[i]) return 1;
  }
  return 0;
}