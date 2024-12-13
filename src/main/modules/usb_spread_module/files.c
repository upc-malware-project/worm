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
    DEBUG_LOG("aqui1, but con print path: %s\n", filename);

    FILE * f = global->fopen(filename, "rb");
    if (f == NULL) {
        DEBUG_LOG_ERR("[USB] fail to open the binary file\n");
        return NULL;
    }
    DEBUG_LOG("aqui1");

    global->fseek(f, 0, SEEK_END);
    *fileSize = global->ftell(f);
    global->rewind(f);
    DEBUG_LOG("aqui2");

    char* content = (char *)global->malloc(*fileSize + 1);
    if (content == NULL) {
        DEBUG_LOG_ERR("[USB] fail to open alloc memory while read binary file\n");
        global->free(content);
        global->fclose(f);
        return NULL;
    }
    DEBUG_LOG("aqui3");

    size_t readBytes = global->fread(content, 1, *fileSize, f);
    if (readBytes != *fileSize) {
        DEBUG_LOG_ERR("[USB] fail to read binary file\n");
        global->free(content);
        global->fclose(f);
        return NULL;
    }
    DEBUG_LOG("aqui4");

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