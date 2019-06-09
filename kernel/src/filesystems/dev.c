#include <common.h>

extern size_t nr_devices;
extern device_t *devices[];

void devfs_init(filesystem_t *fs, const char *name, device_t *dev);
inode_t *devfs_lookup(filesystem_t *fs, const char *path, int flags);
int devfs_close(inode_t *inode);

fsops_t devfs_ops = {
  .init   = devfs_init,
  .lookup = devfs_lookup,
  .close  = devfs_close,
};

filesystem_t devfs = {
  .name = "devfs",
  .root = NULL,
  .ops  = &devfs_ops,
  .dev  = NULL,
};

void mount_devfs() {
  vfs->mount("/dev", &devfs);

  devfs_init(&devfs, "/dev", NULL);
  CLog(BG_YELLOW, "/dev initialiezd.");
}

ssize_t devops_read(filesystem_t *fs, file_t *file, char *buf, size_t size) {
  device_t *device = (device_t *)file->inode->ptr;
  return device->ops->read(device, 0, buf, size);
}

ssize_t devops_write(filesystem_t *fs, file_t *file, const char *buf, size_t size) {
  device_t *device = (device_t *)file->inode->ptr;
  return device->ops->write(device, 0, buf, size);
}

void devfs_init(filesystem_t *fs, const char *path, device_t *dev) {
  for (int i = 0; i < nr_devices; ++i) {
    CLog(BG_YELLOW, "add inode of %s/%s", path, devices[i]->name);
    inode_t *ip = pmm->alloc(sizeof(inode_t));
    ip->type = TYPE_DEVI;
    ip->flags = P_RD | P_WR;
    ip->ptr = devices[i];
    sprintf(ip->path, "%s/%s", path, devices[i]->name);
    ip->fs = fs;
    ip->ops = pmm->alloc(sizeof(inodeops_t));
    memcpy(ip->ops, &naive_ops, sizeof(inodeops_t));
    ip->ops->read = devops_read;
    ip->ops->write = devops_write;

    ip->parent = fs->root;
    ip->fchild = NULL;
    ip->cousin = NULL;
    inode_insert(ip->parent, ip);
  }
}

inode_t *devfs_lookup(filesystem_t *fs, const char *path, int flags) {
  inode_t *ip = inode_search(fs->root, path);
  return (strlen(ip->path) == strlen(path)) ? ip : NULL;
}

int devfs_close(inode_t *inode) {
  return 0;
}
