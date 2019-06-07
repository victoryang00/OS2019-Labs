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
  devfs_root.parent = &devfs_root;
  devfs_root.fchild = NULL;
  devfs_root.cousin = NULL;
  sprintf(devfs_root.path, "/");
  vfs->mount(path, fs);
}

inode_t *devfs_lookup(filesystem_t *fs, const char *path, int flags) {
  inode_t *ip = inode_search(&devfs_root, path);
  if (!strcmp(path, ip->path)) return ip;
  else {
    CLog(FG_YELLOW, "inode %s not found. create a new one.", path);
    device_t *device = dev_lookup(path + 1);
    if (!device) {
      CLog(FG_YELLOW, "device not found. creation failed.");
      return NULL;
    }

    inode_t *dev_ip = pmm->alloc(sizeof(inode_t));
    inodeops_t *ops = pmm->alloc(sizeof(inodeops_t));
    ops->read = devfs_read;
    ops->write = devfs_write;

    dev_ip->refcnt = 0;
    dev_ip->ptr = (void *)device;
    sprintf(dev_ip->path, "%s", path);
    dev_ip->fs = &devfs;
    dev_ip->ops = ops;
    return dev_ip;
  }
}

int devfs_close(inode_t *inode) {
  return 0;
}
