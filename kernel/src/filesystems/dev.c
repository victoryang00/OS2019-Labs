#include <common.h>

void devfs_init();
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
  vfs->mount("/dev", &devfs);
}

void devfs_init() {
  devfs_root.parent = &devfs_root;
  devfs_root.fchild = NULL;
  devfs_root.cousin = NULL;
  sprintf(devfs_root.path, "/");
}

inode_t *devfs_lookup(filesystem_t *fs, const char *path, int flags) {
  inode_t *ip = inode_search(&devfs_root, path);
  if (!strcpy(path, ip->path)) return ip;
  else {
    CLog(FG_YELLOW, "inode not found. create a new one.");
    device_t *dev = dev_lookup(dev_name + 1);
    if (!dev) {
      CLog(FG_YELLOW, "device not found. creation failed.");
      return NULL
    }

    inode_t dev_ip = pmm->alloc(sizeof(inode_t));
    ops = pmm->alloc(sizeof(inodeops_t));
    ops->read = dev->ops->read;
    ops->write = dev->ops->write;

    dev_ip->refcnt = 0;
    dev_ip->ptr = NULL;
    sprintf(dev_ip->path, "%s", dev_name);
    dev_ip->fs = &devfs;
    dev_ip->ops = ops;
    return dev_ip;
  }
}

int devfs_close(inode_t *inode) {
  return 0;
}
