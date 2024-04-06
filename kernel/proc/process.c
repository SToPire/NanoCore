#include "proc/process.h"
#include "common/cpu.h"
#include "common/elf.h"
#include "common/errno.h"
#include "common/klog.h"
#include "common/macro.h"
#include "common/type.h"
#include "common/x86.h"
#include "fs/tarfs.h"
#include "mm/layout.h"
#include "mm/mm.h"
#include "mm/mmu.h"
#include "proc/schedule.h"
#include "proc/vm.h"
#include "utils/list.h"
#include "utils/util.h"

struct process *proc_tbl[NPROC];
void iret_from_trapframe(); // defined in switch.S

void switch_to_uproc(struct process *proc) {
  // struct process itself is stored at bottom of kstack
  get_cur_cpu()->ts.rsp0 = (u64)proc + PAGE_SIZE;
  lcr3((paddr_t)proc->pgtbl);
}

u8 init_code[] = {
    0xBF, 0x01, 0x00, 0x00, 0x00, // mov rdi, 0x1
    0xBE, 0x0C, 0x00, 0x10, 0x00, // mov rsi, %str
    0xCD, 0x80,                   // int 0x80
    0x2F, 0x69, 0x6E, 0x69, 0x74, // str: "/init"
};

void uproc_init() {
  vaddr_t kpage;
  paddr_t upage;
  struct process *proc;
  vaddr_t sp;

  kpage = P2V(kzalloc(PAGE_SIZE));
  proc = (struct process *)kpage;

  proc_tbl[0] = proc;
  proc->pid = 0;
  proc->parent = proc;
  proc->status = PROC_READY;
  proc->canary = KSTACK_CANARY;
  init_mm_struct(&proc->mm);

  // set trapframe (for iret)
  sp = kpage + PAGE_SIZE;
  sp -= sizeof(struct trap_frame);
  proc->tf = (struct trap_frame *)sp;
  proc->tf->cs = UCODE_SEG_SELECTOR;
  proc->tf->ss = UDATA_SEG_SELECTOR;
  proc->tf->rip = USER_CODE_START;
  // TODO: give a separate user stack
  proc->tf->rsp = USER_STACK_END;
  proc->tf->eflags = EFLAG_IF;

  // set upgtbl
  proc->pgtbl = (ptp_t *)kzalloc(PAGE_SIZE);
  set_kmapping(proc->pgtbl, false);
  upage = kzalloc(PAGE_SIZE);

  // manually map user code of the first user process
  map_one_page(proc->pgtbl, USER_CODE_START, upage, PTE_USER | PTE_WRITE,
               false);

  add_vma(&proc->mm, USER_CODE_START, USER_CODE_START + PAGE_SIZE,
          VM_READ | VM_WRITE | VM_EXEC | VM_USER);
  add_vma(&proc->mm, USER_STACK_START, USER_STACK_END,
          VM_READ | VM_WRITE | VM_USER);

  for (int i = 0; i < sizeof(init_code); i++)
    *(u8 *)P2V(upage + i) = init_code[i];

  // set context
  sp -= sizeof(struct context);
  proc->ctx = (struct context *)sp;
  memset(proc->ctx, 0, sizeof(struct context));
  proc->ctx->rip = (u64)&iret_from_trapframe;
}

struct process *get_cur_proc() {
  // TODO: need pushcli when multiprocessor
  struct process *p;
  p = get_cur_cpu()->cur_proc;
  return p;
}

void yield() {
  struct process *proc = get_cur_proc();

  BUG_ON(proc->status != PROC_RUNNING);
  proc->status = PROC_READY;
  enter_schedule();
}

u32 elf_flag2perm(u32 flag) {
  u32 perm = 0;
  if (flag & PF_W) perm |= PTE_WRITE;
  if (!(flag & PF_X)) perm |= PTE_NONEXEC;

  return perm;
}

u64 elf_flag2vm(u32 flag) {
  u64 vm = VM_READ;
  if (flag & PF_W) vm |= VM_WRITE;
  if (flag & PF_X) vm |= VM_EXEC;

  return vm;
}

void exec(const char *path) {
  int ret;
  struct elf_header *elf_hdr;
  struct elf_program_header *phdr;
  struct vm_area_struct *vma;
  struct process *proc = get_cur_proc();
  vaddr_t va;
  paddr_t pa;
  vaddr_t buf = P2V(kalloc(4 * PAGE_SIZE));

  ret = tarfs_read(path, 0, sizeof(struct elf_header), (void *)buf);
  BUG_ON(ret < 0);

  elf_hdr = (struct elf_header *)buf;
  phdr = (struct elf_program_header *)(buf + sizeof(struct elf_header));

  for (u16 i = 0; i < elf_hdr->e_phnum; ++i) {
    ret = tarfs_read(path, elf_hdr->e_phoff + i * elf_hdr->e_phentsize,
                     elf_hdr->e_phentsize, phdr);
    BUG_ON(ret < 0);

    if (phdr->p_type != PT_LOAD) continue;

    kdebug("[exec] loading segment: vaddr=0x%lx, memsz=0x%lx, filesz=0x%lx, "
           "perm=0x%x\n",
           phdr->p_vaddr, phdr->p_memsz, phdr->p_filesz, phdr->p_flags);

    // TODO: free first user page here ?
    // TODO: Use CoW
    pa = kzalloc(ROUND_UP(phdr->p_memsz, PAGE_SIZE));
    ret = tarfs_read(path, phdr->p_offset, phdr->p_filesz, (void *)P2V(pa));
    BUG_ON(ret < 0);
    memset((void *)P2V(pa) + phdr->p_filesz, 0, phdr->p_memsz - phdr->p_filesz);

    for (va = ROUND_DOWN(phdr->p_vaddr, PAGE_SIZE);
         va < phdr->p_vaddr + phdr->p_memsz; va += PAGE_SIZE, pa += PAGE_SIZE) {
      map_one_page(proc->pgtbl, va, pa, PTE_USER | elf_flag2perm(phdr->p_flags),
                   false);
    }
    add_vma(&proc->mm, ROUND_DOWN(phdr->p_vaddr, PAGE_SIZE),
            ROUND_UP(phdr->p_vaddr + phdr->p_memsz, PAGE_SIZE),
            VM_USER | elf_flag2vm(phdr->p_flags));
  }

  for_each_in_list(vma, struct vm_area_struct, list, &proc->mm.mmap_list) {
    kdebug("[exec] vma: 0x%lx - 0x%lx, flags=0x%lx\n", vma->start, vma->end,
           vma->flags);
  }

  lcr3((paddr_t)proc->pgtbl);
  proc->tf->rip = elf_hdr->e_entry;
}