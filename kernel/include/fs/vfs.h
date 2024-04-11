#pragma once

#include "common/type.h"

#define MAX_NAME_LEN 256

struct super_block {
  struct fs_operations* s_op;
  void* s_private;
};

struct inode {
  u64 i_ino;
  u64 i_size;
  struct super_block* i_sb;
  void* i_private;
};

struct dentry {
  char d_name[MAX_NAME_LEN];
  struct inode* d_inode;
};

struct file {
  struct inode* f_inode;
  u64 f_pos;
  u64 f_flags;
};

struct fs_operations {
  struct file* (*open)(const char* path, int flags);
  u64 (*read)(struct file* file, char* buf, u64 size);
  u64 (*write)(struct file* file, const char* buf, u64 size);
  void (*close)(struct file* file);
};

#define O_RDONLY 0
#define O_WRONLY 1
#define O_RDWR 2

struct file* vfs_open(const char* path, int flags);

u64 vfs_read(struct file* file, void* buf, u64 size);
u64 vfs_write(struct file* file, const void* buf, u64 size);
void vfs_close(struct file* file);

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

u64 vfs_lseek(struct file* file, s64 offset, int whence);

void fs_init();