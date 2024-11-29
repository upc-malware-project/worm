#pragma once
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG 1

#define DEBUG_LOG(args...) global->fprintf(global->stderr, args)

#define PRINT_ERROR()                                                          \
  global->printf("[E] %s:%s:%d %s\n", __FILE__, __func__, __LINE__, global->strerror(-1)) // idea how to use errno here...

#define CHECK(e)                                                            \
  if ((e)) {                                                                 \
    PRINT_ERROR();                                                           \
    global->exit(1);                                                                 \
  }
