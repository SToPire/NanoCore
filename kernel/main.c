#include "include/string.h"
#include "include/uart.h"

int main() {
  init_serial();
  printk("\n[INFO] sos booting...\n");
  while (1)
    ;
}