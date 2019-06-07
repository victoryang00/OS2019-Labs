#include <common.h>

extern int nr_devices;
extern device_t **devices;
inode_t devfs_root;

void devfs_init(filesystem_t *fs, const char *name, device_t *dev);
inode_t *devfs_lookup(filesystem_t *fs, const char *path, int flags);
int devfs_close(inode_t *inode);

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

ssize_t devops_read(file_t *file, char *buf, size_t size) {
  device_t *device = (device_t *)file->inode->ptr;
  return device->ops->read(device, 0, buf, size);
}

ssize_t devops_write(file_t *file, const char *buf, size_t size) {
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

  for (int i = 0; i < 8; ++i) {
    inode_t *ip = pmm->alloc(sizeof(inode_t));
    Log("OK1 %p", ip);
    //ip->type = TYPE_DEVI;
    //ip->ptr = devices[i];
    //sprintf(ip->path, "%s/%s", path, devices[i]->name);
    //ip->fs = fs;
    //ip->ops = pmm->alloc(sizeof(inodeops_t));
    Log("OK2 %p", ip->ops);
    continue;
    memcpy(ip->ops, &common_ops, sizeof(inodeops_t));
    ip->ops->read = devops_read;
    ip->ops->write = devops_write;

    ip->parent = &devfs_root;
    ip->fchild = NULL;
    ip->cousin = NULL;
    inode_insert(ip->parent, ip);
  }
}

inode_t *devfs_lookup(filesystem_t *fs, const char *path, int flags) {
  inode_t *ip = inode_search(&devfs_root, path);
  return (strlen(ip->path) == strlen(path)) ? ip : NULL;
}

int devfs_close(inode_t *inode) {
  return 0;
}
