#include "syscall/syscall.h"
#include "common/errno.h"
#include "common/klog.h"
#include "common/type.h"
#include "dev/uart.h"
#include "proc/process.h"

typedef u64 (*syscall_func_t)(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5,
                              u64 arg6);
syscall_func_t syscall_table[N_SYSCALL] = {
    [SYSCALL_EXEC] = sys_exec,
    [SYSCALL_WRITE] = sys_write,
    [SYSCALL_READ] = sys_read,
};

u64 syscall_entry(struct trap_frame* tf) {
  u64 syscall_no = tf->rax;
  u64 arg1 = tf->rdi;
  u64 arg2 = tf->rsi;
  u64 arg3 = tf->rdx;
  u64 arg4 = tf->rcx;
  u64 arg5 = tf->r8;
  u64 arg6 = tf->r9;
  syscall_func_t syscall_func = syscall_table[syscall_no];

  if (syscall_func)
    return syscall_func(arg1, arg2, arg3, arg4, arg5, arg6);
  BUG("Unknown syscall number!");
}

u64 sys_exec(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6) {
  exec((const char*)arg1);  // TODO: is it good to use a user mode ptr?
  return 0;
}

u64 sys_write(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6) {
  int fd = arg1;
  const void* buf = (const void*)arg2;
  int count = arg3;

  struct process* proc = get_cur_proc();
  struct file* file;

  if (fd < 0 || fd >= proc->ofiles.of_count)
    return -EINVAL;

  if (fd == 0) {
    return -EINVAL;
  } else if (fd == 1 || fd == 2) {
    for (int i = 0; i < count; i++) {
      uart_write(((const char*)buf)[i]);
    }
    return count;
  }

  file = proc->ofiles.ofs[fd];
  return vfs_write(file, buf, count);
}

u64 sys_read(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6) {
  int fd = arg1;
  void* buf = (void*)arg2;
  int count = arg3;

  struct process* proc = get_cur_proc();
  struct file* file;

  if (fd < 0 || fd >= proc->ofiles.of_count)
    return -EINVAL;

  if (fd == 1 || fd == 2) {
    return -EINVAL;
  } else if (fd == 0) {
    for (int i = 0; i < count; i++) {
      ((char*)buf)[i] = uart_read();
    }
    return count;
  }

  file = proc->ofiles.ofs[fd];
  return vfs_read(file, buf, count);
}