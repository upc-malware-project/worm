#pragma once

char* readfile(const char* filename);
char *readVirtualFile(const char *filename);
char *readBinaryFile(const char* filename, size_t *size);

bool isSymLink(const char* filename);

char *followSymLink(char* filename);

int writeFile(char* fileContent, const char* destDir, const char *fileName);
int writeBinaryFile(char* fileContent, const char* destDir, const char *fileName, size_t fileSize);

struct dirent **listDir(const char *path, int *count);

bool fileExists(const char *path);

int getUSBSysDevices(char devices[][256], int max_devices);

int compareDeviceLists(char old_devices[][256], int old_count, char new_devices[][256], int new_count);