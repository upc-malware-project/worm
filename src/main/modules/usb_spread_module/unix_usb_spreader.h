#pragma once
#include <stdbool.h>
#include "globals.h"
#include "utils.h"

int usb_spread_module(Globals * glob);

typedef struct Mounts {
    char device[30];
    char mountPoint[300];
    char fsType[30]; //use enum
    char permissions[200]; //use enum, rw or ro
    int dummy1;
    int dummy2;
} Mounts;

typedef struct Node {
    Mounts mount;
    struct Node* next;
    struct Node* prev;
} Node;

