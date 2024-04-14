#include "fs/tarfs.h"
#include "common/errno.h"
#include "common/klog.h"
#include "common/macro.h"
#include "common/type.h"
#include "dev/ata.h"
#include "fs/vfs.h"
#include "mm/layout.h"
#include "mm/mm.h"
#include "utils/util.h"

#define TAR_ALIGNMENT (512)

struct tarfs_super_block tsb;
int tarfs_ino_cnt;

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

struct file* tarfs_open(const char* path, int flags) {
  void* ptr = (void*)tsb.tarfs_base;
  struct tar_header* tar_hdr = (struct tar_header*)ptr;
  struct file* file;
  struct inode* inode;
  struct tarfs_inode* tinode;
  size_t size;

  if (*path == '/')
    ++path;

  while (memcmp((const char*)tar_hdr->magic, TAR_MAGIC, 6) == 0) {
    size = __tar_get_size(tar_hdr->size);
    if (strcmp(path, (const char*)tar_hdr->name) == 0) {
      file = (struct file*)P2V((struct file*)kalloc(sizeof(struct file)));
      if (file == NULL) {
        kerror("tarfs_open: failed to allocate memory for file\n");
        return NULL;
      }

      inode = (struct inode*)P2V((struct inode*)kalloc(sizeof(struct inode)));
      if (inode == NULL) {
        kerror("tarfs_open: failed to allocate memory for inode\n");
        kfree(V2P(file));
        return NULL;
      }

      tinode = (struct tarfs_inode*)P2V(
          (struct tarfs_inode*)kalloc(sizeof(struct tarfs_inode)));
      if (tinode == NULL) {
        kerror("tarfs_open: failed to allocate memory for tarfs_inode\n");
        kfree(V2P(file));
        kfree(V2P(inode));
        return NULL;
      }

      tinode->offset = (size_t)tar_hdr - (size_t)tsb.tarfs_base;

      inode->i_size = size;
      inode->i_ino = tarfs_ino_cnt++;
      inode->i_sb = tsb.sb;
      inode->i_private = tinode;

      file->f_inode = inode;
      file->f_flags = flags;
      file->f_pos = 0;

      return file;
    }
    ptr += sizeof(struct tar_header);      // skip file metadata
    ptr += ROUND_UP(size, TAR_ALIGNMENT);  // skip file data
    tar_hdr = (struct tar_header*)ptr;
  }

  return NULL;
}

u64 tarfs_read(struct file* file, char* buf, u64 size) {
  struct inode* inode = file->f_inode;
  struct tarfs_inode* tinode = inode->i_private;
  void* data;

  if (file->f_pos + size > inode->i_size) {
    size = inode->i_size - file->f_pos;
  }

  if (size == 0) {
    return 0;
  }

  data = (void*)((size_t)tsb.tarfs_base + tinode->offset +
                 sizeof(struct tar_header));
  memcpy(buf, data + file->f_pos, size);

  file->f_pos += size;

  return size;
}

u64 tarfs_write(struct file* file, const char* buf, u64 size) {
  return -ESUPPORT;
}

void tarfs_close(struct file* file) {
  kfree(V2P(file->f_inode->i_private));
  kfree(V2P(file->f_inode));
  kfree(V2P(file));
}

struct fs_operations tarfs_ops = {
    .open = tarfs_open,
    .read = tarfs_read,
    .write = tarfs_write,
    .close = tarfs_close,
};

struct super_block* tarfs_mount() {
  struct super_block* sb =
      (struct super_block*)P2V(kalloc(sizeof(struct super_block)));
  if (sb == NULL) {
    kerror("tarfs_init: failed to allocate memory for super block\n");
    BUG_ON(true);
  }
  // TODO: fix me
  tsb.tarfs_len = TARFS_SECTCNT * ATA_SECTOR_SIZE;
  tsb.tarfs_base = P2V(kalloc(tsb.tarfs_len));
  for (int i = 0; i < TARFS_SECTCNT; i += 16) {
    ata_readsectn((void*)((size_t)tsb.tarfs_base + i * ATA_SECTOR_SIZE), false,
                  i, 16);
  }

  sb->s_op = &tarfs_ops;
  sb->s_private = &tsb;

  return sb;
}