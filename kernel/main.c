#include "include/string.h"
#include "include/uart.h"

int main() {
  init_serial();
  printk("\nsos booting...\n");
  while (1)
    ;
}