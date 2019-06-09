#include <common.h>
#include <file.h>
#include <vfs.h>

int error_open(filesystem_t *fs, file_t *file, int flags) {
  return E_BADFS;
}

int error_close(filesystem_t *fs, file_t *file) {
  return E_BADFS;
}

ssize_t error_read(filesystem_t *fs, file_t *file, char *buf, size_t size) {
  return E_BADFS;
}

ssize_t error_write(filesystem_t *fs, file_t *file, const char *buf, size_t size) {
  return E_BADFS;
}

off_t error_lseek(filesystem_t *fs, file_t *file, off_t offset, int whence) {
  return E_BADFS;
}

int error_mkdir(filesystem_t *fs, const char *path) {
  return E_BADFS;
}

int error_rmdir(filesystem_t *fs, const char *path) {
  return E_BADFS;
}

int error_link(filesystem_t *fs, const char *path, inode_t *inode) {
  return E_BADFS;
}

int error_unlink(filesystem_t *fs, const char *path) {
  return E_BADFS;
}

int error_readdir(filesystem_t *fs, inode_t *inode, char *ret) {
  return naive_readdir(fs, inode, ret);
}

inodeops_t error_ops = {
  .open    = error_open,
  .close   = error_close,
  .read    = error_read,
  .write   = error_write,
  .lseek   = error_lseek,
  .mkdir   = error_mkdir,
  .rmdir   = error_rmdir,
  .link    = error_link,
  .unlink  = error_unlink,
  .readdir = error_readdir,
};
