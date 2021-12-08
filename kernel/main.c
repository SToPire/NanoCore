#include "include/klog.h"
#include "include/string.h"
#include "include/uart.h"

int main() {
  init_serial();
  printk("\n");
  kinfo("sos booting...\n");
  while (1)
    ;
}