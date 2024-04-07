#pragma once

#include "common/cpu.h"
#include "common/type.h"

#define SYSCALL_EXEC (1)
#define N_SYSCALL (32)

u64 syscall_entry(struct trap_frame* tf);

u64 sys_exec(u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5);
