#include <common.h>

void devfs_init() {
  return;
}

inode_t *devfs_lookup(filesystem_t *fs, const char *path, int flags) {
  return NULL;
}

int devfs_close(inode_t *inode) {
  return 0;
}

fsops_t devfs_ops = {
  devfs_init,
  devfs_lookup,
  devfs_close,
};

filesystem_t devfs = {
  &devfs_ops, NULL,
};

void mount_devfs() {
  vfs->mount("/dev", &devfs);
}
