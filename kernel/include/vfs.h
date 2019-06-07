#ifndef __VFS_H__
#define __VFS_H__

#include <common.h>
#include <amdev.h>
#include <klib.h>
#include <devices.h>

struct fsops;
struct filesystem;

typedef struct fsops fsops_t;
typedef struct filesystem filesystem_t;

struct fsops {
  void (*init)(filesystem_t *fs, const char *name, device_t *dev);
  inode_t *(*lookup)(filesystem_t *fs, const char *path, int flags);
  int (*close)(inode_t *inode);
};

struct filesystem {
  fsops_t *ops;
  device_t *dev;
};

#endif
