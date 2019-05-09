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

struct DataSeg {
  void *head;
  void *tail;
  struct DataSeg *prev;
  struct DataSeg *next;
};

struct Image {
  char name[128];
  char ext[8];
  struct BMP bmp;
  struct Image *next;
};

void recover_images(struct Disk *);
struct Image *read_image(struct FDT *);
void output_image(struct Image *);

#endif
