## Bootloader

Bootloader由MBR(Master Boot Record)和loader组成。

MBR位于硬盘的第0扇区（512字节），负责：

1. 关闭在BIOS中开启的中断
2. 将栈基址设置在`0x7c00`处
3. 开启A20地址线，设置32位GDT，进入保护模式
4. 准备identity mapping的64位页表，开启分页
5. 设置64位GDT，进入Long Mode

6. 从硬盘中读取loader到内存（`0x8000`）并调用

Loader从硬盘第1扇区始，并占据若干个扇区，负责：

1. 解析内核elf文件，并根据其header从硬盘将内核载入内存
2. 修改页表以映射内存：`[VM_OFFSET, VM_OFFSET] => [0, 1GB]`，使内核代码能访问到所在的低地址空间。
3. 调用内核入口点



## Kernel

### 内存分配器

内存分配器包括两部分：

- Phase1 allocator，由若干4K页通过链表串联成的dummy allocator，主要用于在内核页表初始化前分配供内核页表使用的内存
- Normal allocator，供正常的内存分配使用。超过`PAGE_SIZE`的分配请求由*buddy allocator*进行，小于`PAGE_SIZE`的分配请求在页内部由slab allocator进行。



### 异常处理

- 创建256个中断向量，负责保存*trapno*，并跳转到统一的中断处理入口点。对于某些异常，还需要保存*errno*，详见手册。
- 用中断向量的地址设置IDT
- 在中断处理入口点处，保存通用寄存器以构建*trap frame*，交给`exception_handler`。
- `exception_handler`根据`trapno`做分发