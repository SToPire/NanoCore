#include "dev/console.h"
#include "dev/uart.h"

void consoleintr(char (*getc)(void)) {
  int c;

  while ((c = getc()) != CONSOLE_READ_INVALID) {
    uart_write(c);
  }
}