#include <common.h>
#include <vfs.h>
#include <devices.h>
#include <threads.h>

mnt_t mnt_head;

static inline mnt_t *find_mnt(const char *path) {
  for (mnt_t *mp = mnt_head->next; mp != &mnt_head; mp = mp->next) {
    if (!strcpy(path, mp->path)) {
      return mp;
    }
  }
  return NULL;
}

static inline inode_t *find_inode_by_path(const char *path) {
  mnt_t *mp = find_mnt(path);
  if (!mp) return NULL;
  return mp->fs->ops->lookup(mp->fs, path + strlen(mp->path), 0);
}

static inline file_t *find_file_by_fd(int fd) {
  task_t *cur = get_current_task();
  return cur->fildes[fd];
}

void vfs_init() {
  mnt_head.next = &mnt_head;
  mnt_head.prev = &mnt_head;
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
  return 0;
}

int vfs_unmount(const char *path) {
  mnt_t *mp = find_mnt(path);
  Assert(mp, "Path %s not mounted!", path);
  mp->prev->next = mp->next;
  mp->next->prev = mp->prev;
  pmm->free(mp);
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
  return 0;
}

int vfs_open(const char *path, int flags) {
  return 0;
}

ssize_t vfs_read(int fd, void *buf, size_t nbyte) {
  file_t *fp = find_file_by_fd(fd);
  return fp->inode->ops->read(fp, buf, nbyte);
}

ssize_t vfs_write(int fd, void *buf, size_t nbyte) {
  file_t *fp = find_file_by_fd(fd);
  return fp->inode->ops->write(fp, buf, nbyte);
}

off_t vfs_lseek(int fd, off_t offset, int whence) {
  file_t *fp = find_file_by_fd(fd);
  return fp->inode->ops->lseek(fp, offset, whence);
}

int vfs_close(int fd) {
  file_t *fp = find_file_by_fd(fd);
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
