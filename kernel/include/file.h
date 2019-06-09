#ifndef __FILE_H__
#define __FILE_H__

#include <common.h>
#include <vfs.h>

#define O_RDONLY 0x01
#define O_WRONLY 0x02
#define O_RDWR   0x03
#define O_CREAT  0x04

#define P_RD O_RDONLY
#define P_WR O_WRONLY

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
  int (*open)(filesystem_t *fs, file_t *file, int flags);
  int (*close)(filesystem_t *fs, file_t *file);
  ssize_t (*read)(filesystem_t *fs, file_t *file, char *buf, size_t size);
  ssize_t (*write)(filesystem_t *fs, file_t *file, const char *buf, size_t size);
  off_t (*lseek)(filesystem_t *fs, file_t *file, off_t offset, int whence);
  int (*mkdir)(filesystem_t *fs, const char *name);
  int (*rmdir)(filesystem_t *fs, const char *name);
  int (*link)(filesystem_t *fs, const char *name, inode_t *inode);
  int (*unlink)(filesystem_t *fs, const char *name);
  int (*readdir)(filesystem_t *fs, inode_t *inode, char *ret);
};
extern inodeops_t naive_ops;

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
  int flags;
  void *ptr;
  int32_t blk;
  char path[256];
  off_t offset;
  size_t size;
  filesystem_t *fs;
  inodeops_t *ops;

  inode_t *parent;
  inode_t *fchild;
  inode_t *cousin;
};
extern inode_t *root;

int naive_open(filesystem_t *fs, file_t *file, int flags);
int naive_close(filesystem_t *fs, file_t *file);
ssize_t naive_read(filesystem_t *fs, file_t *file, char *buf, size_t size);
ssize_t naive_write(filesystem_t *fs, file_t *file, const char *buf, size_t size);
off_t naive_lseek(filesystem_t *fs, file_t *file, off_t offset, int whence);
int naive_mkdir(filesystem_t *fs, const char *path);
int naive_rmdir(filesystem_t *fs, const char *path);
int naive_link(filesystem_t *fs, const char *path, inode_t *inode);
int naive_unlink(filesystem_t *fs, const char *path);
int naive_readdir(filesystem_t *fs, inode_t *inode, char *ret);

inode_t *inode_search(inode_t *cur, const char *path);
void inode_insert(inode_t *parent, inode_t *child);
void inode_remove(inode_t *parent, inode_t *child);
void inode_delete(inode_t *cur);

#endif
