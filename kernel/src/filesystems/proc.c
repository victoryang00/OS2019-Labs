#include <common.h>

extern task_t root_task;

void procfs_init(filesystem_t *fs, const char *name, device_t *dev);
inode_t *procfs_lookup(filesystem_t *fs, const char *path, int flags);
int procfs_close(inode_t *inode);

fsops_t procfs_ops = {
  .init   = procfs_init,
  .lookup = procfs_lookup,
  .close  = procfs_close,
};

filesystem_t procfs = {
  .ops = &procfs_ops,
  .dev = NULL,
};

void mount_procfs() {
  vfs->mount("/proc", &procfs);

  procfs_init(&procfs, "/proc", NULL);
  CLog(BG_YELLOW, "/proc initialiezd.");
}

ssize_t procops_read(file_t *file, char *buf, size_t size) {
  sprintf(buf, "read not implemented!!");
}

void procfs_init(filesystem_t *fs, const char *path, device_t *dev) {
}

inode_t *procfs_lookup(filesystem_t *fs, const char *path, int flags) {
  inode_t *ip = inode_search(procfs.root, path);
  return (strlen(ip->path) == strlen(path)) ? ip : NULL;
}

int procfs_close(inode_t *inode) {
  return 0;
}
