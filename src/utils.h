#pragma once
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG 1

#define DEBUG_LOG(args...) fprintf(stderr, args)

#define PRINT_ERROR()                                                          \
  printf("[E] %s:%s:%d %s\n", __FILE__, __func__, __LINE__,               \
         strerror(errno))

#define CHECK(e)                                                            \
  do {                                                                         \
    if ((e)) {                                                                 \
      PRINT_ERROR();                                                           \
      exit(1);                                                                 \
    }                                                                          \
  } while (0)
