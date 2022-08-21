#include "syscall/syscall.h"
#include "common/klog.h"
#include "common/type.h"

typedef u64 (*syscall_func_t)(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5);
syscall_func_t syscall_table[N_SYSCALL] = {
    [SYSCALL_EXEC] = sys_exec,
};

u64 syscall_entry(struct trap_frame *tf) {
  u64 syscall_no = tf->rdi;
  u64 arg1 = tf->rsi;
  u64 arg2 = tf->rdx;
  u64 arg3 = tf->rcx;
  u64 arg4 = tf->r8;
  u64 arg5 = tf->r9;
  syscall_func_t syscall_func = syscall_table[syscall_no];

  if (syscall_func) return syscall_func(arg1, arg2, arg3, arg4, arg5);
  BUG("Unknown syscall number!");
}

u64 sys_exec(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5) {
  exec((const char *)arg1); // TODO: is it good to use a user mode ptr?
  return 0;
}
