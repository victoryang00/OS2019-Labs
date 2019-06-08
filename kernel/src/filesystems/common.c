#include <common.h>
#include <file.h>
#include <vfs.h>

const char *inode_types_human[] = {
  "INVL",
  "MNTP",
  "DIRC",
  "FILE",
  "DEVI",
  "PROC",
  "PROX",
  "LINK",
};

inode_t *root;

inodeops_t common_ops = {
  .open    = common_open,
  .close   = common_close,
  .read    = common_read,
  .write   = common_write,
  .lseek   = common_lseek,
  .mkdir   = common_mkdir,
  .rmdir   = common_rmdir,
  .link    = common_link,
  .unlink  = common_unlink,
  .readdir = common_readdir,
};

fsops_t commonfs_ops = {
  .init   = commonfs_init,
  .lookup = commonfs_lookup,
  .close  = commonfs_close,
};

filesystem_t commonfs = {
  .ops = &commonfs_ops,
  .dev = NULL,
};

int common_open(file_t *file, int flags) {
  return 0;
}

int common_close(file_t *file) {
  return 0;
}

ssize_t common_read(file_t *file, char *buf, size_t size) {
  Assert(file->inode->fs->dev, "fs with no device");
  device_t *dev = file->inode->fs->dev;
  off_t offset = (off_t)file->inode->ptr + file->inode->offset;
  if (offset > file->inode->size) {
    buf[0] = '\0';
    return 0;
  } else {
    ssize_t ret = dev->ops->read(dev, offset, buf, size);
    file->inode->offset += (off_t)ret;
    return ret;
  }
}

ssize_t common_write(file_t *file, const char *buf, size_t size) {
  Assert(file->inode->fs->dev, "fs with no device");
  device_t *dev = file->inode->fs->dev;
  off_t offset = (off_t)file->inode->ptr + file->inode->offset;
  ssize_t ret = dev->ops->write(dev, offset, buf, size);
  file->inode->offset += (off_t)ret;
  if (file->inode->offset > file->inode->size) file->inode->size = (size_t)file->inode->offset;
  return ret;
}

off_t common_lseek(file_t *file, off_t offset, int whence) {
  switch (whence) {
    case SEEK_SET:
      file->inode->offset = offset;
      break;
    case SEEK_CUR:
      file->inode->offset += offset;
      break;
    case SEEK_END:
    default:
      file->inode->offset = file->inode->size + offset;
      break;
  }
  return file->inode->offset;
}

int common_mkdir(const char *name) {
  return 0;
}

int common_rmdir(const char *name) {
  inode_t *ip = inode_search(root, name);
  if (ip->type != TYPE_DIRC) {
    return -1;
  }
  inode_delete(ip);
  return 0;
}

int common_link(const char *name, inode_t *inode) {
  inode_t *pp = inode_search(root, name);
  if (strlen(pp->path) == strlen(name)) {
    return -1;
  }

  inode_t *ip = pmm->alloc(sizeof(inode_t));
  ip->refcnt = 0;
  ip->type = TYPE_LINK;
  ip->ptr = (void *)inode;
  sprintf(ip->path, name);
  ip->offset = 0;
  ip->size = sizeof(void *);
  ip->fs = pp->fs;
  ip->ops = pmm->alloc(sizeof(inodeops_t));
  memcpy(ip->ops, &common_ops, sizeof(inodeops_t));

  ip->parent = pp;
  ip->fchild = NULL;
  ip->cousin = NULL;
  inode_insert(pp, ip);
  return 0;
}

int common_unlink(const char *name) {
  inode_t *ip = inode_search(root, name);
  if (ip->type != TYPE_FILE || ip->type != TYPE_LINK) {
    return -1;
  }
  inode_delete(ip);
  return 0;
}

void common_readdir(inode_t *inode, char *ret) {
  sprintf(ret, "ls %s:\n", inode->path);
  strcat(ret, " + TYPE PRIV FILENAME\n");
  for (inode_t *ip = inode->fchild; ip != NULL; ip = ip->cousin) {
    CLog(FG_PURPLE, "%s %s", inode_types_human[ip->type], ip->path);
    strcat(ret, " - ");
    strcat(ret, inode_types_human[ip->type]);
    strcat(ret, " ");

    if (ip->type == TYPE_DIRC || ip->type == TYPE_MNTP) {
      strcat(ret, "D --");
    } else {
      strcat(ret, "- ");
      strcat(ret, ip->flags & P_RD ? "R" : "-");
      strcat(ret, ip->flags & P_WR ? "W" : "-");
    }
    strcat(ret, " ");

    strcat(ret, ip->path);
    strcat(ret, "\n");
  }
}

void commonfs_init(filesystem_t *fs, const char *path, device_t *dev) {
  fs->dev = dev;

  root = pmm->alloc(sizeof(inode_t));
  root->type = TYPE_MNTP;
  root->ptr = NULL;
  sprintf(root->path, path);
  root->fs = fs;
  root->ops = &common_ops;
  root->parent = root;
  root->fchild = NULL;
  root->cousin = NULL;
}

inode_t *commonfs_lookup(filesystem_t *fs, const char *path, int flags) {
  inode_t *ip = inode_search(root, path);
  if (strlen(ip->path) == strlen(path)) {
    return ip;
  } else {
    if (flags & O_CREAT) {
      size_t len = strlen(path);
      for (size_t i = strlen(ip->path) + strlen("/"); i < len; ++i) {
        if (path[i] == '/') return NULL;
      }
      
      inode_t *pp = ip;
      ip = pmm->alloc(sizeof(inode_t));
      ip->refcnt = 0;
      ip->type = TYPE_FILE;
      ip->ptr = NULL;
      ip->size = 0;
      ip->fs = fs;
      ip->ops = pmm->alloc(sizeof(inodeops_t));
      memcpy(ip->ops, &common_ops, sizeof(inodeops_t));

      ip->parent = pp;
      ip->fchild = NULL;
      ip->cousin = NULL;
      inode_insert(pp, ip);
      return ip;
    } else {
      return NULL;
    }
  }
}

int commonfs_close(inode_t *inode) {
  if (inode->size <= 0) {
    inode_remove(inode->parent, inode);
  }
  return 0;
}
