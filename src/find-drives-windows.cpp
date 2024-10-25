#include "find-drives-windows.h"
#include <cosmo.h>
#include <libc/nt/files.h>
#include <libc/nt/createfile.h>
#include <libc/nt/enum/filesharemode.h>
#include <libc/nt/enum/creationdisposition.h>
#include <string>
#include "utils.h"
#include "constants.h"
#include "structs.h"

// https://stackoverflow.com/questions/21243073/identify-connected-drives-in-windows
// https://doxygen.reactos.org/d1/d25/dll_2win32_2kernel32_2client_2file_2disk_8c_source.html
int getUsbDrives() {
  int drives = GetLogicalDrives();

  for (int drive = 0; drive < MAX_DOS_DRIVES; drive++) {
    if (drives & (1 << drive)) {
      char16_t deviceName[7] = u"\\\\.\\A:";
      deviceName[ARRAYLEN(deviceName) - 3] += drive;

      int handle = CreateFile(deviceName, 0,
                              kNtFileShareRead | kNtFileShareWrite,
                              nullptr,
                              kNtOpenExisting,
                              0,
                              0);

      if (const char16_t *error = getErrorString()) {
        printf("%hs\n", error);
        return -1;
      }

      printf("found drive %hs %d\n", deviceName, handle);

      DWORD bytes;
      STORAGE_DEVICE_DESCRIPTOR devd;
      STORAGE_PROPERTY_QUERY query;
      memset(&query, 0, sizeof(query));
      query.PropertyId = StorageDeviceProperty;
      query.QueryType = PropertyStandardQuery;

      if (!DeviceIoControl(handle,
                           IOCTL_STORAGE_QUERY_PROPERTY,
                           &query, sizeof(query),
                           &devd, sizeof(devd),
                           &bytes, NULL)) {
        printf("%hs\n", getErrorString());
        return -1;
      }

      printf("type %hs %d\n", deviceName, devd.BusType);
    }
  }
  return 0;
}