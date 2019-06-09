#ifndef __VFS_H__
#define __VFS_H__

#include <common.h>
#include <amdev.h>
#include <klib.h>
#include <devices.h>

#ifdef VFS_DEBUG
#define VFSLog(...)  Log(__VA_ARGS__)
#define VFSCLog(...) CLog(__VA_ARGS__)
#else
#define VFSLog(...) 
#define VFSCLog(...)
#endif

struct fsops {
  void (*init)(filesystem_t *fs, const char *path, device_t *dev);
  inode_t *(*lookup)(filesystem_t *fs, const char *path, int flags);
  int (*close)(inode_t *inode);
};
extern fsops_t naivefs_ops;

struct filesystem {
  const char *name;
  inode_t *root;
  fsops_t *ops;
  device_t *dev;
};
extern filesystem_t naivefs;

typedef struct mnt_table {
  const char *path;
  filesystem_t *fs;
  struct mnt_table *next;
  struct mnt_table *prev;
} mnt_t;

void naivefs_init(filesystem_t *fs, const char *name, device_t *dev);
inode_t *naivefs_lookup(filesystem_t *fs, const char *path, int flags);
int naivefs_close(inode_t *inode);

#endif
