#include <common.h>
#include <vfs.h>
#include <devices.h>
#include <threads.h>

mnt_t mnt_head;

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
  return ret;
}

inline inode_t *find_inode_by_path(const char *path) {
  mnt_t *mp = find_mnt(path);
  if (!mp) return NULL;
  return mp->fs->ops->lookup(mp->fs, path + strlen(mp->path), 0);
}

inline file_t *find_file_by_fd(int fd) {
  task_t *cur = get_current_task();
  return cur->fildes[fd];
}

void vfs_init() {
  mnt_head.next = &mnt_head;
  mnt_head.prev = &mnt_head;

  extern void mount_devfs();
  mount_devfs();
}

int vfs_access(const char *path, int mode) {
  return 0;
}

int vfs_mount(const char *path, filesystem_t *fs) {
  Assert(!find_mnt(path), "Path %s already mounted!", path);
  mnt_t *mp = pmm->alloc(sizeof(mnt_t));
  mp->path = path;
  mp->fs = fs;
  mp->prev = mnt_head.prev;
  mp->next = &mnt_head;
  mnt_head.prev = mp;
  mp->prev->next = mp;

  CLog(BG_YELLOW, "Path %s is mounted.", path);
  return 0;
}

int vfs_unmount(const char *path) {
  mnt_t *mp = find_mnt(path);
  Assert(mp, "Path %s not mounted!", path);
  mp->prev->next = mp->next;
  mp->next->prev = mp->prev;
  pmm->free(mp);
  CLog(BG_YELLOW, "Path %s is unmounted.", path);
  return 0;
}

int vfs_mkdir(const char *path) {
  return 0;
}

int vfs_rmdir(const char *path) {
  return 0;
}

int vfs_link(const char *oldpath, const char *newpath) {
  return 0;
}

int vfs_unlink(const char *path) {
  mnt_t *mp = find_mnt(path);
  Assert(mp, "Path %s is not mounted!", path);
  inode_t *ip = mp->fs->ops->lookup(mp->fs, path + strlen(mp->path), 0);
  Assert(ip, "Inode %s is not found!", path + strlen(mp->path));
  ip->ops->unlink(ip->path);
  return 0;
}

int vfs_open(const char *path, int flags) {
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
  inode_t *ip = mp->fs->ops->lookup(mp->fs, path + strlen(mp->path), flags);
  Assert(ip, "Inode %s is not found!", path + strlen(mp->path));

  fp = pmm->alloc(sizeof(file_t));
  fp->fd = fd;
  fp->refcnt = 0;
  fp->flags = flags;
  fp->inode = ip;
  fp->offset = 0;

  cur->fildes[fd] = fp;
  return fd;
}
 
ssize_t vfs_read(int fd, void *buf, size_t nbyte) {
  file_t *fp = find_file_by_fd(fd);
  Assert(fp, "file pointer is NULL");
  return fp->inode->ops->read(fp, buf, nbyte);
}

ssize_t vfs_write(int fd, void *buf, size_t nbyte) {
  file_t *fp = find_file_by_fd(fd);
  Assert(fp, "file pointer is NULL");
  return fp->inode->ops->write(fp, buf, nbyte);
}

off_t vfs_lseek(int fd, off_t offset, int whence) {
  file_t *fp = find_file_by_fd(fd);
  Assert(fp, "file pointer is NULL");
  return fp->inode->ops->lseek(fp, offset, whence);
}

int vfs_close(int fd) {
  file_t *fp = find_file_by_fd(fd);
  Assert(fp, "file pointer is NULL");
  return fp->inode->ops->close(fp);
}

MODULE_DEF(vfs) {
  .init    = vfs_init,
  .access  = vfs_access,
  .mount   = vfs_mount,
  .unmount = vfs_unmount,
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
