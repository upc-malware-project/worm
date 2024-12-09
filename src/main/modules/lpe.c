#include "lpe.h"
#include <stdio.h>
#include <unistd.h>

void try_get_root() {
  if (geteuid() == 0) {
    printf("Running as root!");
    return;
  }

  printf("Trying to gain root");
  return;
}
