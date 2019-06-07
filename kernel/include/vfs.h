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

MODULE {
  void (*init)();
  int (*access)(const char *path, int mode);
  int (*mount)(const char *path, filesystem_t *fs);
  int (*unmount)(const char *path);
  int (*mkdir)(const char *path);
  int (*rmdir)(const char *path);
  int (*link)(const char *oldpath, const char *newpath);
  int (*unlink)(const char *path);
  int (*open)(const char *path, int flags);
  ssize_t (*read)(int fd, void *buf, size_t nbyte);
  ssize_t (*write)(int fd, void *buf, size_t nbyte);
  off_t (*lseek)(int fd, off_t offset, int whence);
  int (*close)(int fd);
} MOD_NAME(vfs);

#endif
