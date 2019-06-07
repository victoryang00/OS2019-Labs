#include <common.h>

void ramfs_init(filesystem_t *fs, const char *name, device_t *dev);
inode_t *ramfs_lookup(filesystem_t *fs, const char *path, int flags);
int ramfs_close(inode_t *inode);

fsops_t ramfs_ops = {
  .init   = ramfs_init,
  .lookup = ramfs_lookup,
  .close  = ramfs_close,
};

filesystem_t ramfs = {
  .ops = &ramfs_ops,
  .dev = NULL,
};

void ramfs_init(filesystem_t *fs, const char *path, device_t *dev) {
  fs->dev = dev;
  root.fs = fs;
  sprintf(root.path, path);
}

inode_t *ramfs_lookup(filesystem_t *fs, const char *path, int flags) {
  inode_t *ip = inode_search(&root, path);
  if (!strcmp(path, ip->path)) return ip;
  else {
    CLog(FG_YELLOW, "inode %s not found. create a new one.", path);
    Panic("ramfs not implemented!");
  }
}

int ramfs_close(inode_t *inode) {
  return 0;
}
