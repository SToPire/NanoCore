#include "common/klog.h"
#include "utils/string.h"
#include "utils/uart.h"
#include "mm/layout.h"

int main() {
  init_serial();
  printk("\n");
  kinfo("sos booting...\n");
  while (1)
    ;
}