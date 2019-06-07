#include <common.h>
#include <file.h>
#include <vfs.h>

inodeops_t common_ops = {
  .open    = common_open,
  .close   = common_close,
  .read    = common_read,
  .write   = common_write,
  .lseek   = common_lseek,
  .mkdir   = common_mkdir,
  .rmdir   = common_rmdir,
  .link    = common_link,
  .unlink  = common_unlink,
  .readdir = common_readdir,
};

int common_open(file_t *file, int flags) {
  return 0;
}

int common_close(file_t *file) {
  return 0;
}

ssize_t common_read(file_t *file, char *buf, size_t size) {
  return 0;
}

ssize_t common_write(file_t *file, const char *buf, size_t size) {
  return 0;
}

off_t common_lseek(file_t *file, off_t offset, int whence) {
  return 0;
}

int common_mkdir(const char *name) {
  return 0;
}

int common_rmdir(const char *name) {
  return 0;
}

int common_link(const char *name, inode_t *inode) {
  return 0;
}

int common_unlink(const char *name) {
  return 0;
}

void common_readdir(inode_t *inode, char *ret) {
  sprintf(ret, "ls %s:\n", inode->path);
  for (inode_t *ip = inode->fchild; ip != NULL; ip = ip->cousin) {
    Log(ip->path);
    strcat(ret, " - ");
    strcat(ret, ip->path);
    strcat(ret, "\n");
  }
}
