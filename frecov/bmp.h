#ifndef __BMP_H__
#define __BMP_H__

#include "frecov.h"
#include <stdint.h>

struct BMP_Header {
  uint16_t type;
  uint32_t size;
  uint16_t reserved1;
  uint16_t reserved2;
  uint32_t offset;
} __attribute__((packed));

struct BMP_Info {
  uint32_t size;
  int32_t  width;  // with sign
  int32_t  height; // with sign
  uint16_t planes;
  uint16_t bit_count;
  uint32_t compression;
  uint32_t image_size;
  int32_t  resolution_x;
  int32_t  resolution_y;
  uint32_t color_used;
  uint32_t color_important;
} __attribute__((packed));

struct BMP {
  struct BMP_Header header;
  struct BMP_Info info;
  void* data;
};

#endif
