NanoCore is a in-progress toy kernel written for x86-64 architecture.

## Progress

- [y] Bootloader
- [y] Paging
- [y] Memory management
- [y] Interrupts
- [?] Process management (half-done)
- [?] VFS (half-done)
- [?] Syscalls (half-done) 
- [n] Keyboard driver
- [n] Shell
- [n] Multitasking
- [n] Multi-core support
- [n] Page cache

## Usage

### Build the kernel

NanoCore uses a docker container to package the build environment.
To build the kernel and the bootloader and generate the kernel image, run the following command:
```
make kernel
```

Currently NanoCore uses a tarball to store the user space binaries. To build the tarball, run the following command:
```
make user
```

Simply running `make` will build both the kernel and the user space binaries.

### Run the kernel

To run the kernel, run the following command:
```
make qemu
```

### Generate docker builder

To generate the builder docker image, run the following command:
```
make gen-builder
```

## Refences

The following resources were used to build NanoCore:

- [OSDev Wiki](https://wiki.osdev.org/Main_Page)
- [xv6](https://github.com/mit-pdos/xv6-public)
- [xv6-riscv](https://github.com/mit-pdos/xv6-riscv)
- [ChCore](https://gitee.com/ipads-lab/chcore-lab)
