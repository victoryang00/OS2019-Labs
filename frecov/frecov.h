#ifndef __COMMON_H__
#define __COMMON_H__

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

#define DEBUG
#include "debug.h"
#include "bmp.h"
#include "fat32.h"

#define FOLDER "./recov"

struct DataSeg {
  void *head;
  struct DataSeg *prev;
  struct DataSeg *next;
};

struct Image {
  char name[128];
  size_t size;
  int clus;
  FILE *file;
  int16_t *chk;
  struct Image *prev;
  struct Image *next;
};

enum ClusterTypes {
  TYPE_FDT, // file entry
  TYPE_BMP, // bmp image
  TYPE_EMP  // empty entry
};

void recover_images();
int get_cluster_type(void *, int);
void handle_fdt(void *, int, bool);
bool handle_fdt_aux(void *, int, bool);
void handle_bmp(void *, size_t);
bool handle_bmp_aux(void *, size_t);
struct Image *find_best_match(void *, size_t);
void output_image(struct Image *);

#endif
