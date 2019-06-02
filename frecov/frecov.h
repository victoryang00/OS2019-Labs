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
#include <sys/wait.h>

//#define DEBUG
#include "debug.h"
#include "bmp.h"
#include "fat32.h"

#define FOLDER "./recov"

struct DataSeg {
  void *head;
  bool eof;
  struct Image *holder;
  struct DataSeg *prev;
  struct DataSeg *next;
};

struct Image {
  char name[128];
  char sha1[128];
  size_t size;
  int clus;
  FILE *file;
  struct BMP *bmp;
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
void handle_image(struct Image *, size_t, int);
void *get_next_cluster(uint8_t *);

#endif
