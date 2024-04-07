#include "common/cpu.h"
#include "common/klog.h"
#include "dev/uart.h"
#include "fs/tarfs.h"
#include "interrupt/interrupt.h"
#include "mm/mm.h"
#include "proc/process.h"
#include "proc/schedule.h"
#include "utils/string.h"

int errno;

void print_welcome() {
  printk("\n");
  kinfo("sos booting...\n");
}

int main() {
  cpu_init();       // init gdt & tss
  uart_init();      // init uart device
  print_welcome();  // show welcome message
  mm_init();        // init memory management module
  intr_init();      // enable interrupt
  tarfs_init();     // use tarfs as disk file system
  uproc_init();     // init first user process
  scheduler();      // enter scheduler loop

  kdebug("spinning!\n");
  while (1)
    ;
}