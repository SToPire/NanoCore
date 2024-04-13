#pragma once

#include "common/cpu.h"
#include "common/type.h"

#define SYSCALL_EXEC (1)
#define SYSCALL_WRITE (2)
#define SYSCALL_READ (3)
#define N_SYSCALL (32)

u64 syscall_entry(struct trap_frame* tf);

u64 sys_exec(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
u64 sys_write(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
u64 sys_read(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6);
