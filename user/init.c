#include "user.h"

int add(int a, int b) {
  return a + b;
}

int main() {
  int a = 2;
  int b = 3;

  int c = add(a, b);

  write(1, "Hello World\n", 12);

  char buf[1];
  read(0, buf, 1);
  write(1, buf, 1);

  while (1)
    ;
  return 0;
}