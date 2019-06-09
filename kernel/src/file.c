#include <common.h>
#include <file.h>

inode_t *inode_search(inode_t *cur, const char *path) {
  CVFSLog(FG_BLUE, "looking for %s from %s", path, cur->path);
  for (inode_t *ip = cur->fchild; ip != NULL; ip = ip->cousin) {
    CVFSLog(FG_BLUE, "%s -> %s", cur->path, ip->path);
    if (!strncmp(path, ip->path, strlen(ip->path))) {
      if (strlen(path) == strlen(ip->path)) return ip;
      else return inode_search(ip, path);
    }
  }
  return cur;
}

void inode_insert(inode_t *parent, inode_t *child) {
  if (parent->fchild) {
    inode_t *ip = parent->fchild;
    while (ip->cousin) ip = ip->cousin;
    ip->cousin = child;
  } else {
    parent->fchild = child;
  }
}

void inode_remove(inode_t *parent, inode_t *child) {
  if (parent->fchild == child) {
    parent->fchild = child->cousin;
  } else {
    inode_t *ip = parent->fchild;
    while (ip->cousin && ip->cousin != child) ip = ip->cousin;
    if (ip->cousin == NULL) return;
    ip->cousin = child->cousin;
  }
}

void inode_delete(inode_t *cur) {
  while (cur->fchild) {
    inode_delete(cur->fchild);
  }
  inode_remove(cur->parent, cur);
}
