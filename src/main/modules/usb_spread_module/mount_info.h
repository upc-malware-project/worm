#pragma once
#include <stdbool.h>
#include <stdlib.h>

#include "globals.h"

typedef struct Mounts {
    char device[30];
    char mountPoint[300];
    char fsType[30]; //use enum
    char permissions[200]; //use enum, rw or ro
    int dummy1;
    int dummy2;
} Mounts;

Mounts* parseMountLine(char* line);

Mounts* parseMountsFile(size_t *arrSize);

bool isUSBMount(char* devPath, char* mountPath);

// To fix "global" variable, issue with loading crashes
void init_mount_info_glob(Globals * glob);