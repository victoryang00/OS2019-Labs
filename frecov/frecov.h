#ifndef __COMMON_H__
#define __COMMON_H__

#ifdef __i386__
#define LU "%lu"
#else
#define LU "%llu"
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
