#ifndef __FILE_H__
#define __FILE_H__

#include <common.h>
#include <vfs.h>

#define O_RDONLY 0x01
#define O_WRONLY 0x02
#define O_RDWR   0x03
#define O_CREAT  0x04

#define SEEK_SET 0x00
#define SEEK_CUR 0x01
#define SEEK_END 0x02

struct file {
  int fd;
  int refcnt;
  int flags;
  inode_t *inode;
  uint64_t offset;
};

struct inodeops {
  int (*open)(file_t *file, int flags);
  int (*close)(file_t *file);
  ssize_t (*read)(file_t *file, char *buf, size_t size);
  ssize_t (*write)(file_t *file, const char *buf, size_t size);
  off_t (*lseek)(file_t *file, off_t offset, int whence);
  int (*mkdir)(const char *name);
  int (*rmdir)(const char *name);
  int (*link)(const char *name, inode_t *inode);
  int (*unlink)(const char *name);
  void (*readdir)(inode_t *inode, char *ret);
};
extern inodeops_t common_ops;

enum INODE_TYPES {
  TYPE_INVL, // invalid
  TYPE_MNTP, // mount point
  TYPE_DIRC, // directory
  TYPE_FILE, // file
  TYPE_DEVI, // device (/dev)
  TYPE_PROC, // process (/proc)
  TYPE_PROX, // special proc
  TYPE_LINK, // linked file
};
extern const char *inode_types_human[];

struct inode {
  int refcnt;
  int type;
  void *ptr;
  char path[256];
  off_t offset;
  size_t size;
  filesystem_t *fs;
  inodeops_t *ops;

  inode_t *parent;
  inode_t *fchild;
  inode_t *cousin;
};
extern inode_t root;

int common_open(file_t *file, int flags);
int common_close(file_t *file);
ssize_t common_read(file_t *file, char *buf, size_t size);
ssize_t common_write(file_t *file, const char *buf, size_t size);
off_t common_lseek(file_t *file, off_t offset, int whence);
int common_mkdir(const char *name);
int common_rmdir(const char *name);
int common_link(const char *name, inode_t *inode);
int common_unlink(const char *name);
void common_readdir(inode_t *inode, char *ret);

inline inode_t *inode_search(inode_t *cur, const char *path) {
  CLog(FG_BLUE, "looking for %s from %s", path, cur->path);
  for (inode_t *ip = cur->fchild; ip != NULL; ip = ip->cousin) {
    CLog(FG_BLUE, "%s -> %s", cur->path, ip->path);
    if (!strncmp(path, ip->path, strlen(ip->path))) {
      if (strlen(path) == strlen(ip->path)) return ip;
      else return inode_search(ip, path);
    }
  }
  return cur;
}

inline void inode_insert(inode_t *parent, inode_t *child) {
  if (parent->fchild) {
    inode_t *ip = parent->fchild;
    while (ip->cousin) ip = ip->cousin;
    ip->cousin = child;
  } else {
    parent->fchild = child;
  }
}

inline void inode_remove(inode_t *parent, inode_t *child) {
  if (parent->fchild == child) {
    parent->fchild = child->cousin;
  } else {
    inode_t *ip = parent->fchild;
    while (ip->cousin && ip->cousin != child) ip = ip->cousin;
    if (ip->cousin == NULL) return;
    ip->cousin = child->cousin;
  }
}

inline void inode_delete(inode_t *);
inline void inode_delete(inode_t *cur) {
  while (cur->fchild) {
    inode_delete(cur->fchild);
  }
  inode_remove(cur->parent, cur);
}

#endif
