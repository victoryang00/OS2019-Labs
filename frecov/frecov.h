#ifndef __COMMON_H__
#define __COMMON_H__

#if defined (__i386__)
  #define FMT_LU "%llu"
#elif defined (__x86_64__)
  #define FMT_LU "%lu"
#endif

#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

#define DEBUG
#include "debug.h"

#include "fat32.h"

#endif
