#include "lib/user.h"

int main() {
  printf("Hello World from user mode!\n");
  exec("/shell");

  return 0;
}