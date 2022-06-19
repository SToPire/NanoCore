#include "common/klog.h"
#include "mm/mm.h"
#include "utils/string.h"
#include "utils/uart.h"

void print_welcome() {
  printk("\n");
  kinfo("sos booting...\n");
}

int main() {
  init_serial();
  print_welcome();

  mm_init();

  while (1)
    ;
}