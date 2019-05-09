#ifndef __COMMON_H__
#define __COMMON_H__

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

#define DEBUG
#include "debug.h"
#include "fat32.h"

void recover_images(struct Disk *);

#endif
