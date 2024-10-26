#include <cosmo.h>
#include <cstdio>
#include "spreader.h"

int main() {
  ShowCrashReports();
  setbuf(stdout, 0);
  fprintf(stdout, "Initialize\n");
  fprintf(stdout, "Windows: %d\n", IsWindows());
  fprintf(stdout, "Linux: %d\n", IsLinux());
  spread();
  return 0;
}