typedef unsigned long size_t;
typedef int bool;

#define NULL ((void*)0)
#define true 1
#define false 0

size_t strlen(const char* s);
int strcmp(const char* s1, const char* s2);
int printf(const char* fmt, ...);

// System call wrappers for user programs
int exec(const char* path);
int write(int fd, const void* buf, int count);
int read(int fd, void* buf, int count);