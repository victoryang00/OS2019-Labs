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

inline ssize_t read_proc(task_t *tp, char *buf, size_t size) {
  return snprintf(buf, size, "Process %d:\n - Name: %s\n - State: %s\n",
      tp->pid, tp->name, task_states_human[tp->state]);
}
ssize_t procops_read(file_t *file, char *buf, size_t size) {
  char *path = file->inode->path;
  if (!strcmp(path, "/proc/self")) {
    task_t *cur = get_current_task();
    return read_proc(cur, buf, size); 
  } else if (!strcmp(path, "/proc/cpuinfo")) {
    return snprintf(buf, size, "CPU info:\n - Cores: %d\n - Model: i996\n", _ncpu());
  } else if (!strcmp(path, "/proc/meminfo")) {
    return snprintf(buf, size, "MEM info:\n - Start: 0x%p\n -   End: 0x%p\n", _heap.start, _heap.end);
  } else {
    return read_proc(file->inode->ptr, buf, size);
  }
}

inline void procfs_self() {
  inode_t *ip = pmm->alloc(sizeof(inode_t));
  ip->type = TYPE_PROX;
  ip->ptr = NULL;
  sprintf(ip->path, "/proc/self");
  ip->fs = &procfs;
  ip->ops = pmm->alloc(sizeof(inodeops_t));
  memcpy(ip->ops, &common_ops, sizeof(inodeops_t));
  ip->ops->read = procops_read;

  ip->parent = procfs.root;
  ip->fchild = NULL;
  ip->cousin = NULL;
  inode_insert(ip->parent, ip);
}

inline void procfs_cpuinfo() {
  inode_t *ip = pmm->alloc(sizeof(inode_t));
  ip->type = TYPE_PROX;
  ip->ptr = NULL;
  sprintf(ip->path, "/proc/cpuinfo");
  ip->fs = &procfs;
  ip->ops = pmm->alloc(sizeof(inodeops_t));
  memcpy(ip->ops, &common_ops, sizeof(inodeops_t));
  ip->ops->read = procops_read;

  ip->parent = procfs.root;
  ip->fchild = NULL;
  ip->cousin = NULL;
  inode_insert(ip->parent, ip);
}

inline void procfs_meminfo() {
  inode_t *ip = pmm->alloc(sizeof(inode_t));
  ip->type = TYPE_PROX;
  ip->ptr = NULL;
  sprintf(ip->path, "/proc/meminfo");
  ip->fs = &procfs;
  ip->ops = pmm->alloc(sizeof(inodeops_t));
  memcpy(ip->ops, &common_ops, sizeof(inodeops_t));
  ip->ops->read = procops_read;

  ip->parent = procfs.root;
  ip->fchild = NULL;
  ip->cousin = NULL;
  inode_insert(ip->parent, ip);
}

void procfs_init(filesystem_t *fs, const char *path, device_t *dev) {
  if (procfs.root->fchild == NULL) {
    // first init, add /self, /cpuinfo, /meminfo
    procfs_self();
    procfs_cpuinfo();
    procfs_meminfo();
  }

  // remove all procs first
  bool succ = true;
  while (succ) {
    succ = false;
    for (inode_t *ip = procfs.root->fchild; ip != NULL; ip = ip->cousin) {
      if (ip->type == TYPE_PROC) {
        inode_remove(procfs.root, ip);
        succ = true;
        break;
      }
    }
  }

  // add all procs again
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
