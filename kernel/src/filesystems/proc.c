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
  return strlen(buf);
}

void procfs_init(filesystem_t *fs, const char *path, device_t *dev) {
  while (procfs.root->fchild != NULL) {
    inode_remove(procfs.root, procfs.root->fchild);
  }

  for (task_t *tp = root_task.next; tp != NULL; tp = tp->next) {
    CLog(BG_YELLOW, "add inode of %s/%d", path, tp->pid);
    inode_t *ip = pmm->alloc(sizeof(inode_t));
    ip->type = TYPE_PROC;
    ip->ptr = (void *)tp;
    sprintf(ip->path, "%s/%d", path, tp->pid);
    ip->fs = fs;
    ip->ops = pmm->alloc(sizeof(inodeops_t));
    memcpy(ip->ops, &common_ops, sizeof(inodeops_t));
    ip->ops->read = procops_read;

    ip->parent = procfs.root;
    ip->fchild = NULL;
    ip->cousin = NULL;
    inode_insert(ip->parent, ip);
  }
}

inode_t *procfs_lookup(filesystem_t *fs, const char *path, int flags) {
  inode_t *ip = inode_search(procfs.root, path);
  return (strlen(ip->path) == strlen(path)) ? ip : NULL;
}

int procfs_close(inode_t *inode) {
  return 0;
}
