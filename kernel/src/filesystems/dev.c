#include <common.h>

void devfs_init(filesystem_t *fs, const char *name, device_t *dev);
inode_t *devfs_lookup(filesystem_t *fs, const char *path, int flags);
int devfs_close(inode_t *inode);

inode_t devfs_root;

fsops_t devfs_ops = {
  .init   = devfs_init,
  .lookup = devfs_lookup,
  .close  = devfs_close,
};

filesystem_t devfs = {
  .ops = &devfs_ops,
  .dev = NULL,
};

void mount_devfs() {
  devfs_init(&devfs, "/dev", NULL);
}

ssize_t devfs_read(file_t *file, char *buf, size_t size) {
  device_t *device = (device_t *)file->inode->ptr;
  return device->ops->read(device, 0, buf, size);
}

ssize_t devfs_write(file_t *file, const char *buf, size_t size) {
  device_t *device = (device_t *)file->inode->ptr;
  return device->ops->write(device, 0, buf, size);
}

void devfs_init(filesystem_t *fs, const char *path, device_t *dev) {
  Assert(!dev, "/dev has no device");
  devfs_root.parent = &root;
  devfs_root.fchild = NULL;
  devfs_root.cousin = NULL;
  sprintf(devfs_root.path, path);
  vfs->mount(path, fs);
}

inode_t *devfs_lookup(filesystem_t *fs, const char *path, int flags) {
  inode_t *ip = inode_search(&devfs_root, path);
  return (strlen(ip->path) == strlen(path)) ? ip : NULL;
}

int devfs_close(inode_t *inode) {
  return 0;
}
