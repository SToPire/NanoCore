#pragma once

#include "common/type.h"

#define TAR_MAGIC "ustar "

struct tar_header {
  u8 name[100];
  u8 mode[8];
  u8 uid[8];
  u8 gid[8];
  u8 size[12];
  u8 mtime[12];
  u8 cksum[8];
  u8 typeflag;
  u8 linkname[100];
  u8 magic[6];
  u8 version[2];
  u8 uname[32];
  u8 gname[32];
  u8 devmajor[8];
  u8 devminor[8];
  u8 prefix[155];
  u8 padding[12];
};

#define TARFS_SECTCNT (16)

void tarfs_init();
int tarfs_read(const char* path, size_t off, int len, void* buf);
