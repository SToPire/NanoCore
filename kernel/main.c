#include "common/klog.h"
#include "common/uart.h"
#include "interrupt/interrupt.h"
#include "mm/mm.h"
#include "utils/string.h"

int errno;

void print_welcome() {
  printk("\n");
  kinfo("sos booting...\n");
}

int main() {
  init_serial();
  print_welcome();
  mm_init();
  intr_init();

  kdebug("spinning!\n");
  while (1)
    ;
}