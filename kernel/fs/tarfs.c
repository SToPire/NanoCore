#include "fs/tarfs.h"
#include "common/errno.h"
#include "common/klog.h"
#include "common/macro.h"
#include "common/type.h"
#include "dev/ata.h"
#include "mm/layout.h"
#include "mm/mm.h"
#include "utils/util.h"

#define TAR_ALIGNMENT (512)

vaddr_t tarfs_base;
u64 tarfs_len;

void tarfs_init() {
  tarfs_len = TARFS_SECTCNT * ATA_SECTOR_SIZE;
  tarfs_base = P2V(kalloc(tarfs_len));

  ata_readsectn((void *)tarfs_base, false, 0, TARFS_SECTCNT);
}

// size is stored in octal format, convert it
size_t __tar_get_size(u8 size[12]) {
  size_t res = 0;

  BUG_ON(size[11] != 0);
  for (int i = 0; i < 11; i++) {
    res <<= 3;
    res += size[i] - '0';
  }

  return res;
}

int tarfs_read(const char *path, size_t off, int len, void *buf) {
  void *ptr = (void *)tarfs_base;
  struct tar_header *tar_hdr = (struct tar_header *)ptr;
  size_t size;

  if (*path == '/') ++path;

  while (memcmp((const char *)tar_hdr->magic, TAR_MAGIC, 6) == 0) {
    size = __tar_get_size(tar_hdr->size);
    if (strcmp(path, (const char *)tar_hdr->name) == 0) {
      if (off > size) return -EINVAL;
      memcpy(buf, ptr + sizeof(struct tar_header) + off, len);
      return len;
    }
    ptr += sizeof(struct tar_header);     // skip file metadata
    ptr += ROUND_UP(size, TAR_ALIGNMENT); // skip file data
    tar_hdr = (struct tar_header *)ptr;
  }
}