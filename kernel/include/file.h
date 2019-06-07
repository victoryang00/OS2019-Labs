#ifndef __FILE_H__
#define __FILE_H__

#include <common.h>
#include <vfs.h>

#define O_RDONLY 0x01
#define O_WRONLY 0x02
#define O_RDWR   0x03

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

#define TYPE_MNT 0
#define TYPE_DIR 1
#define TYPE_FIL 2

struct inode {
  int refcnt;
  int type;
  void *ptr;
  const char *path;
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
  for (inode_t *ip = cur->fchild; ip != NULL; ip = ip->cousin) {
    Log("%s -> %s", cur->path, ip->path);
    if (!strncmp(path, ip->path, strlen(ip->path))) {
      if (strlen(path) == strlen(ip->path)) return ip;
      else return inode_search(ip, path + strlen(ip->path));
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

#endif
