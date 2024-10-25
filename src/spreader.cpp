#include <cosmo.h>
#include <cstdio>
#include <libc/dce.h>
#include "find-drives-windows.h"

void spread() {
  printf("spread init\n");
  if (IsWindows()) {
    getUsbDrives();
  } else {

  }
}