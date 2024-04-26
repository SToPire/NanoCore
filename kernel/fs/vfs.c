#include "fs/vfs.h"
#include "common/errno.h"
#include "fs/tarfs.h"

// TODO: tmp
struct super_block* root_fs;

struct file* vfs_open(const char* path, int flags) {
  // TODO: re-open of the same file should have the same inode
  return root_fs->s_op->open(path, flags);
}

void vfs_close(struct file* file) {
  root_fs->s_op->close(file);
}

size_t vfs_read(struct file* file, void* buf, size_t count) {
  return root_fs->s_op->read(file, buf, count);
}

size_t vfs_write(struct file* file, const void* buf, size_t count) {
  return root_fs->s_op->write(file, buf, count);
}

u64 vfs_lseek(struct file* file, s64 offset, int whence) {
  struct inode* inode = file->f_inode;
  u64 size = inode->i_size;

  if (whence == SEEK_SET) {
    if (offset < 0 || offset > size) {
      return -EINVAL;
    }
    file->f_pos = offset;
  } else if (whence == SEEK_CUR) {
    u64 new_pos = file->f_pos + offset;
    if (new_pos < 0 || new_pos > size) {
      return -EINVAL;
    }
    file->f_pos = new_pos;
  } else if (whence == SEEK_END) {
    if (offset > 0 || -offset > size) {
      return -EINVAL;
    }
    file->f_pos = size + offset;
  } else {
    return -EINVAL;
  }

  return file->f_pos;
}

void fs_init() {
  root_fs = tarfs_mount();
}