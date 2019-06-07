#ifndef __FILE_H__
#define __FILE_H__

#include <common.h>

struct file_t;
struct inodeops_t;
struct inode_t;

typedef struct file {
  int refcnt;
  inode_t *inode;
  uint64_t offset;
} file_t;

typedef struct inodeops {
  int (*open)(file_t *file, int flags);
  int (*close)(file_t *file);
  ssize_t (*read)(file_t *file, char *buf, size_t size);
  ssize_t (*write)(file_t *file, const char *buf, size_t size);
  off_t (*lseek)(file_t *file, off_t offset, int whence);
  int (*mkdir)(const char *name);
  int (*rmdir)(const char *name);
  int (*link)(const char *name, inode_t *inode);
  int (*unlink)(const char *name);
} inodeops_t;

typedef struct inode {
  int refcnt;
  void *ptr;
  filesystem_t *fs;
  inodeops_t *ops;
} inode_t;

#endif
