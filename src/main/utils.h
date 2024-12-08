#pragma once
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "globals.h"

#define DEBUG 1

#define DEBUG_LOG(args...) if(DEBUG) global->fprintf(global->stdout, args)
#define DEBUG_LOG_ERR(args...) if(DEBUG) global->fprintf(global->stderr, args)

#define PRINT_ERROR()                                                          \
  global->printf("[E] %s:%s:%d %s\n", __FILE__, __func__, __LINE__, global->strerror(-1)) // idea how to use errno here...

#define CHECK(e)                                                            \
  if ((e)) {                                                                 \
    PRINT_ERROR();                                                           \
    global->exit(1);                                                                 \
  }

#define ASSERT(e, what)                                                            \
  if (!(e)) {                                                                 \
    DEBUG_LOG_ERR("[E] (%s) %s:%s:%d %s\n", what, __FILE__, __func__, __LINE__);                                                           \
    global->exit(1);                                                                 \
  }

#define TRAP if(0){\
                 __asm__("int $3"); \
              }

// fooling linear sweep
#define FOOLS __asm__(                                                  \
        ".intel_syntax noprefix\n"                                      \
        ".byte 0xeb, 0x8\n"                                             \
        ".byte 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 \n"       \
        ".att_syntax\n"                                                 \
    );

#define MAX_PATH_LEN 256
#define KEYLEN 8

static Globals *global;

void load_file_bytes(Globals *global);
uint64_t generate_key(Globals *global);
void mutate_lib(Globals* global);

