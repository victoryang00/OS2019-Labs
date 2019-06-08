#ifndef __VFS_H__
#define __VFS_H__

#include <common.h>
#include <amdev.h>
#include <klib.h>
#include <devices.h>

struct fsops {
  void (*init)(filesystem_t *fs, const char *path, device_t *dev);
  inode_t *(*lookup)(filesystem_t *fs, const char *path, int flags);
  int (*close)(inode_t *inode);
};
extern fsops_t commonfs_ops;

struct filesystem {
  const char *name;
  inode_t *root;
  fsops_t *ops;
  device_t *dev;
};
extern filesystem_t commonfs;

typedef struct mnt_table {
  const char *path;
  filesystem_t *fs;
  struct mnt_table *next;
  struct mnt_table *prev;
} mnt_t;

void commonfs_init(filesystem_t *fs, const char *name, device_t *dev);
inode_t *commonfs_lookup(filesystem_t *fs, const char *path, int flags);
int commonfs_close(inode_t *inode);

#endif
