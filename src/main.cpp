#include <cosmo.h>
#include <cstdio>
#include "spreader.h"

int main() {
  printf("Initialize\n");
  printf("Windows: %d\n", IsWindows());
  printf("Linux: %d\n", IsLinux());

  spread();
  return 0;
}