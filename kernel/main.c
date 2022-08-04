#include "common/cpu.h"
#include "common/klog.h"
#include "interrupt/interrupt.h"
#include "mm/mm.h"
#include "proc/process.h"
#include "utils/string.h"
#include "utils/uart.h"

int errno;

void print_welcome() {
  printk("\n");
  kinfo("sos booting...\n");
}

int main() {
  cpu_init();      // init gdt & tss
  uart_init();     // init uart device
  print_welcome(); // show welcome message
  mm_init();       // init memory management module
  intr_init();     // enable interrupt
  uproc_init();    // init first user process

  kdebug("spinning!\n");
  while (1)
    ;
}