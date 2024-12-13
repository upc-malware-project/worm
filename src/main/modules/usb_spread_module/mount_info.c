#include "mount_info.h"
#include <unistd.h>
#include "mount_info_node.h"
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "files.h"
#include "string_utils.h"

#include "utils.h"

bool isUSBMount(char* devPath, char* mountPath) {
    char* pathToMatch = "/dev/sd";

    char *whereIsIt = findSubstring(devPath, pathToMatch);
    if (whereIsIt != NULL && devPath == whereIsIt) {
        //contains pathToMatch, and it's at the beginning

        char rootDir[2] = "/";
        char homeDir[6] = "/home";
        char *isHomeDir = findSubstring(mountPath, homeDir);
        if(global->strncmp(mountPath, rootDir, sizeof(rootDir)) == 0 || isHomeDir != NULL) {
            return false;
        }
        return true;
    }
    
    return false;
}

Mounts* parseMountLine(char* line) {
    Mounts* mountInfo = (Mounts *) global->malloc(sizeof(Mounts));
    int i = 0;
    char* rawMountLineInfo = global->strtok(line, " ");
    while (rawMountLineInfo != NULL) {
        i++;
        switch (i) {
        case 1:
            global->strncpy(mountInfo->device, rawMountLineInfo, sizeof(mountInfo->device)); //for now ignore if atoi return 0
            break;
        case 2:
            global->strncpy(mountInfo->mountPoint, rawMountLineInfo, sizeof(mountInfo->mountPoint));
            break;
        case 3:
            global->strncpy(mountInfo->fsType, rawMountLineInfo, sizeof(mountInfo->fsType));
            break;
        case 4:
            global->strncpy(mountInfo->permissions, rawMountLineInfo, sizeof(mountInfo->permissions));
            break;
        case 5:
            mountInfo->dummy1 = global->atoi(rawMountLineInfo);
            break;
        case 6:
            mountInfo->dummy2 = global->atoi(rawMountLineInfo);
            break;
        }
        rawMountLineInfo = global->strtok(NULL, " ");
    }
    
    return mountInfo;
}

Mounts* parseMountsFile(size_t *arrSize) {
    char *procMountPath = "/proc/mounts";
    if (!fileExists(procMountPath)) {
        procMountPath = "/etc/mtab";
    }

    while (isSymLink(procMountPath)){
        char *resolvedProcMountPath = procMountPath;
        procMountPath = followSymLink(resolvedProcMountPath);
    }

    char* fileContent = readVirtualFile(procMountPath);
    if (!fileContent) {
        DEBUG_LOG_ERR("[USB] fail to read virtual file\n");
        return NULL;
    }

    char *lineCtx;
    char* line = global->__strtok_r(fileContent, "\n", &lineCtx);
    Node *headSysMounts = NULL;
    Node *tailSysMounts = NULL;
    while (line != NULL) {
        Mounts *mountInfo = parseMountLine(line);
        if (mountInfo == NULL) {
            DEBUG_LOG_ERR("[USB] invalid mount line parsed, should not happen\n");
        } else {
            appendNode(&headSysMounts, &tailSysMounts, *mountInfo);
        }
        line = global->__strtok_r(NULL, "\n", &lineCtx);
    }

    Mounts* mounts = listToArr(headSysMounts, tailSysMounts, arrSize);
    
    return mounts;
}