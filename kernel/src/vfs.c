#include <common.h>
#include <vfs.h>
#include <devices.h>
#include <threads.h>

inode_t *root;
mnt_t mnt_head, mnt_root;
sem_t vfs_sem;
bool after_init = false;

inline mnt_t *find_mnt(const char *path) {
  size_t max_match = 0;
  mnt_t *ret = NULL;
  for (mnt_t *mp = mnt_head.next; mp != &mnt_head; mp = mp->next) {
    if (!strncmp(path, mp->path, strlen(mp->path))) {
      if (strlen(mp->path) > max_match) {
        max_match = strlen(mp->path);
        ret = mp;
      }
    }
  }
  VFSLog("%s is mounted by %s", path, ret->path);
  return ret;
}

inline file_t *find_file_by_fd(int fd) {
  task_t *cur = get_current_task();
  return cur->fildes[fd];
}

void vfs_init() {
  kmt->sem_init(&vfs_sem, "vfs-lock", 0);

  root = pmm->alloc(sizeof(inode_t));
  root->refcnt = 0;
  root->type = TYPE_MNTP;
  root->flags = P_RD;
  root->ptr = NULL;
  sprintf(root->path, "/");
  root->offset = 0;
  root->size = 4;
  root->fs = &naivefs;
  root->ops = &naive_ops;
  root->parent = root;
  root->fchild = NULL;
  root->cousin = NULL;
  naivefs.root = root;

  mnt_root.path = "/";
  mnt_root.fs = &naivefs;
  mnt_root.next = &mnt_head;
  mnt_root.prev = &mnt_head;
  mnt_head.next = &mnt_root;
  mnt_head.prev = &mnt_root;

  extern void mount_naivefs();
  mount_naivefs();
  
  extern void mount_devfs();
  mount_devfs();

  extern void mount_procfs();
  mount_procfs();

  after_init = true;
}

int vfs_access(const char *path, int mode) {
  Log("access");
  kmt->sem_wait(&vfs_sem);

  mnt_t *mp = find_mnt(path);
  Assert(mp, "Path %s not mounted!", path);
  inode_t *ip = inode_search(root, path);

  int ret = 0;
  if (strlen(ip->path) == strlen(path)) {
    if ((mode & ip->flags) != (mode & ~O_CREAT)) {
      ret = E_BADPR;
    }
  } else {
    if (mode & O_CREAT) {
      size_t len = strlen(path);
      for (size_t i = strlen(ip->path) + 1; i < len; ++i) {
        if (path[i] == '/') {
          ret = E_NOENT;
          break;
        }
      }
    } else {
      ret = E_NOENT;
    }
  }
  kmt->sem_signal(&vfs_sem);
  return ret;
}

int vfs_mount(const char *path, filesystem_t *fs) {
  Log("mount");
  if (after_init) kmt->sem_wait(&vfs_sem);

  mnt_t *mp = find_mnt(path);
  Assert(!mp || strlen(mp->path) != strlen(path), "Path %s already mounted!", path);
  
  mp = pmm->alloc(sizeof(mnt_t));
  mp->path = path;
  mp->fs = fs;
  mp->prev = mnt_head.prev;
  mp->next = &mnt_head;
  mnt_head.prev = mp;
  mp->prev->next = mp;

  inode_t *pp = inode_search(root, path);
  Assert(strcmp(path, pp->path), "inode %s exists!", path);
  fs->root->parent = pp;
  inode_insert(pp, fs->root);
  VFSCLog(BG_YELLOW, "Path %s is mounted.", path);

  if (after_init) kmt->sem_signal(&vfs_sem);
  return 0;
}

int vfs_unmount(const char *path) {
  Log("unmount");
  kmt->sem_wait(&vfs_sem);

  mnt_t *mp = find_mnt(path);
  Assert(mp, "Path %s not mounted!", path);
  mp->prev->next = mp->next;
  mp->next->prev = mp->prev;
  pmm->free(mp);
  VFSCLog(BG_YELLOW, "Path %s is unmounted.", path);

  kmt->sem_signal(&vfs_sem);
  return 0;
}

int vfs_readdir(const char *path, void *buf) {
  Log("readdir");
  kmt->sem_wait(&vfs_sem);

  mnt_t *mp = find_mnt(path);
  Assert(mp, "Path %s not mounted!", path);
  inode_t *ip = mp->fs->ops->lookup(mp->fs, path, O_RDONLY);
  if (!ip) {
    kmt->sem_signal(&vfs_sem);
    return E_NOENT;
  }

  VFSLog("inode has fs %s", mp->fs->name);
  int ret = ip->ops->readdir(mp->fs, ip, (char *)buf);

  kmt->sem_signal(&vfs_sem);
  return ret;
}

int vfs_mkdir(const char *path) {
  Log("mkdir");
  kmt->sem_wait(&vfs_sem);

  mnt_t *mp = find_mnt(path);
  Assert(mp, "Path %s not mounted!", path);
  int ret = mp->fs->root->ops->mkdir(mp->fs, path);

  kmt->sem_signal(&vfs_sem);
  return ret;
}

int vfs_rmdir(const char *path) {
  Log("rmdir");
  kmt->sem_wait(&vfs_sem);

  mnt_t *mp = find_mnt(path);
  Assert(mp, "Path %s not mounted!", path);
  int ret = mp->fs->root->ops->rmdir(mp->fs, path);

  kmt->sem_signal(&vfs_sem);
  return ret;
}

int vfs_link(const char *oldpath, const char *newpath) {
  Log("link");
  kmt->sem_wait(&vfs_sem);

  mnt_t *mp = find_mnt(oldpath);
  Assert(mp, "Path %s not mounted!", oldpath);
  inode_t *old_ip = mp->fs->ops->lookup(mp->fs, oldpath, O_RDWR);
  int ret = mp->fs->root->ops->link(mp->fs, newpath, old_ip);

  kmt->sem_signal(&vfs_sem);
  return ret;
}

int vfs_unlink(const char *path) {
  Log("unlink");
  kmt->sem_wait(&vfs_sem);

  mnt_t *mp = find_mnt(path);
  Assert(mp, "Path %s not mounted!", path);
  int ret = mp->fs->root->ops->unlink(mp->fs, path);

  kmt->sem_signal(&vfs_sem);
  return ret;
}

int vfs_open(const char *path, int flags) {
  Log("open");
  int precheck = vfs_access(path, flags);
  if (precheck) return precheck;
  Log("open-2");
  kmt->sem_wait(&vfs_sem);

  task_t *cur = get_current_task();
  int fd = -1;
  file_t *fp = NULL;
  for (int i = 0; i < NR_FILDS; ++i) {
    if (!cur->fildes[i]) {
      fd = i;
      break;
    }
  }
  Assert(fd != -1, "No fd is available.");
  
  mnt_t *mp = find_mnt(path);
  Assert(mp, "Path %s is not mounted!", path);
  inode_t *ip = mp->fs->ops->lookup(mp->fs, path, flags);
  if (!ip) {
    kmt->sem_signal(&vfs_sem);
    return E_NOENT;
  }
  if (ip->type == TYPE_DIRC || ip->type == TYPE_MNTP) {
    kmt->sem_signal(&vfs_sem);
    return E_BADTP;
  }

  fp = pmm->alloc(sizeof(file_t));
  fp->fd = fd;
  fp->refcnt = 0;
  fp->flags = flags;
  fp->inode = ip;
  fp->offset = 0;

  int status = ip->ops->open(mp->fs, fp, flags);
  kmt->sem_signal(&vfs_sem);
  if (status) {
    return status;
  } else {
    cur->fildes[fd] = fp;
    return fd;
  }
}
 
ssize_t vfs_read(int fd, void *buf, size_t nbyte) {
  Log("read");
  kmt->sem_wait(&vfs_sem);

  file_t *fp = find_file_by_fd(fd);
  Assert(fp, "file pointer is NULL");
  int ret = fp->inode->ops->read(fp->inode->fs, fp, buf, nbyte);

  kmt->sem_signal(&vfs_sem);
  return ret;
}

ssize_t vfs_write(int fd, void *buf, size_t nbyte) {
  Log("write");
  kmt->sem_wait(&vfs_sem);

  file_t *fp = find_file_by_fd(fd);
  Log("write-1");
  Assert(fp, "file pointer is NULL");
  int ret = fp->inode->ops->write(fp->inode->fs, fp, buf, nbyte);
  Log("write-2");

  kmt->sem_signal(&vfs_sem);
  Log("write-3");
  return ret;
}

off_t vfs_lseek(int fd, off_t offset, int whence) {
  kmt->sem_wait(&vfs_sem);

  file_t *fp = find_file_by_fd(fd);
  Assert(fp, "file pointer is NULL");
  int ret = fp->inode->ops->lseek(fp->inode->fs, fp, offset, whence);

  kmt->sem_signal(&vfs_sem);
  return ret;
}

int vfs_close(int fd) {
  kmt->sem_wait(&vfs_sem);

  file_t *fp = find_file_by_fd(fd);
  Assert(fp, "file pointer is NULL");
  int ret = fp->inode->ops->close(fp->inode->fs, fp);

  task_t *cur = get_current_task();
  cur->fildes[fd] = NULL;

  kmt->sem_signal(&vfs_sem);
  return ret;
}

MODULE_DEF(vfs) {
  .init    = vfs_init,
  .access  = vfs_access,
  .mount   = vfs_mount,
  .unmount = vfs_unmount,
  .readdir = vfs_readdir,
  .mkdir   = vfs_mkdir,
  .rmdir   = vfs_rmdir,
  .link    = vfs_link,
  .unlink  = vfs_unlink,
  .open    = vfs_open,
  .read    = vfs_read,
  .write   = vfs_write,
  .lseek   = vfs_lseek,
  .close   = vfs_close,
};
