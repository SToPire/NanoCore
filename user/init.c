#include "user.h"

int add(int a, int b) {
  return a + b;
}

int main() {
  int a = 2;
  int b = 3;

  int c = add(a, b);

  write(1, "Hello World\n", 12);

  while (1)
    ;
  return 0;
}