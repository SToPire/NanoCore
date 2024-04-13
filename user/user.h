// System call wrappers for user programs
int exec(const char* path);
int write(int fd, const void* buf, int count);
int read(int fd, void* buf, int count);