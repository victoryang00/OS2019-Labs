#include <common.h>
#include <file.h>
#include <vfs.h>

typedef struct commonfs_params {
  int32_t blk_size;
  int32_t map_head;
  int32_t data_head;
  int32_t min_free;
} commonfs_params_t;

typedef union commonfs_entry {
  char content[32];
  struct {
    int32_t head;
    int16_t type;
    int16_t flags;
    char path[24];
  };
} commonfs_entry_t;

int32_t commonfs_get_next_blk(filesystem_t *fs, int32_t blk) {
  int32_t ret = 0;
  commonfs_params_t *params = (commonfs_params_t *)fs->root->ptr;
  off_t offset = params->map_head + blk * sizeof(int32_t);
  fs->dev->ops->read(fs->dev, offset, (void *)(&ret), sizeof(int32_t));
  return ret;
}

int32_t commonfs_get_last_entry_blk(filesystem_t *fs) {
  int32_t blk = 1;
  while (true) {
    int32_t next = commonfs_get_next_blk(fs, blk);
    if (next == 0) return blk;
  }
}

void commonfs_put_params(filesystem_t *fs) {
  fs->dev->ops->write(fs->dev, 0, (void *)fs->ptr, sizeof(commonfs_params_t));
}

void commonfs_add_map(filesystem_t *fs, int32_t from, int32_t to) {
  commonfs_params_t *params = (commonfs_params_t *)fs->root->ptr;
  *(params->map_head + from) = to;
}

commonfs_entry_t commonfs_get_entry(filesystem_t *fs, int32_t blk) {
  commonfs_entry_t ret;
  commonfs_params_t *params = (commonfs_params_t *)fs->root->ptr;
  off_t offset = params->data_head + blk * params->blk_size;
  fs->dev->ops->read(fs->dev, offset, (void *)(&ret), sizeof(commonfs_entry_t));
  return ret;
}

void commonfs_put_entry(filesystem_t *fs, int32_t blk, commonfs_entry_t *entry) {
  commonfs_params_t *params = (commonfs_params_t *)fs->root->ptr;
  off_t offset = params->data_head + blk * params->blk_size;
  fs->dev->ops->write(fs->dev, offset, (void *)entry, sizeof(commonfs_entry_t));
}

void commonfs_add_entry(filesystem_t *fs, commonfs_entry_t *entry) {
  commonfs_params_t *params = (commonfs_params_t *)fs->root->ptr;
  int32_t last = commonfs_get_last_entry_blk(fs);
  int32_t blk = params->min_free;
  ++params->min_free;
  commonfs_add_map(fs, last, blk);
  commonfs_put_entry(fs, blk, entry);
  commonfs_put_params(fs, params);
}

size_t commonfs_get_file_size(filesystem_t *fs, commonfs_entry_t *entry) {
  size_t ret = 0;
  int32_t blk = entry->head;
  commonfs_params_t *params = (commonfs_params_t *)fs->root->ptr;
  while (true) {
    int32_t next = commonfs_get_next_blk(fs, blk);
    if (next) {
      ret += params->blk_size;
    } else {
      commonfs_entry_t ce = commonfs_get_entry(fs, blk);
      for (size_t i = 0; i < params->blk_size; ++i) {
        if (!ce.content[i]) break;
        ++ret;
      }
      break;
    }
    blk = next;
  }
  return ret;
}

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
  .name = "commonfs",
  .root = NULL,
  .ops  = &commonfs_ops,
  .dev  = NULL,
};

int common_open(filesystem_t *fs, file_t *file, int flags) {
  return 0;
}

int common_close(filesystem_t *fs, file_t *file) {
  return 0;
}

ssize_t common_read(filesystem_t *fs, file_t *file, char *buf, size_t size) {
  commonfs_params_t *params = (commonfs_params_t *)fs->root->ptr;
  off_t offset = file->inode->offset;
  int32_t blk = (int32_t)file->inode->ptr;
  
  while (offset >= params->blk_size) {
    offset -= params->blk_size;
    blk = commonfs_get_next_blk(fs, blk);
    if (blk == 0) return 0;
  }

  ssize_t nread = 0;
  while (blk != 0 && size > 0) {
    commonfs_entry_t entry = commonfs_get_entry(fs, blk);
    ssize_t delta = 0;
    if (params->blk_size - offset >= size) {
      delta = snprintf(buf + nread, size, entry.content + offset);
    } else {
      delta = snprintf(buf + nread, params->blk_size - offset, entry.content + offset);
    }
    if (delta == 0) break;
    size -= delta;
    nread += delta;
    
    offset = 0;
    blk = commonfs_get_next_blk(fs, blk);
  }
  return nread;
}

ssize_t common_write(filesystem_t *fs, file_t *file, const char *buf, size_t size) {
  Panic("not ready!");
}

off_t common_lseek(filesystem_t *fs, file_t *file, off_t offset, int whence) {
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

int common_mkdir(filesystem_t *fs, const char *path) {
  inode_t *pp = inode_search(fs->root, path);
  if (strlen(pp->path) == strlen(path)) return -1;

  commonfs_entry_t entry = {
    .head = 0x00000000,
    .type = TYPE_DIRC,
    .flags = P_RD | P_WR,
  };
  snprintf(entry.path, 24, path);
  commonfs_add_entry(fs, &entry);
  
  inode_t *ip = pmm->alloc(sizeof(inode_t));
  ip->refcnt = 0;
  ip->type = TYPE_DIRC;
  ip->flags = P_RD | P_WR;
  sprintf(ip->path, path);
  ip->offset = 0;
  ip->size = 4;
  ip->fs = fs;
  ip->ops = pmm->alloc(sizeof(inodeops_t));
  memcpy(ip->ops, &common_ops, sizeof(inodeops_t));

  ip->parent = pp;
  ip->fchild = NULL;
  ip->cousin = NULL;
  inode_insert(pp, ip);
  return 0;
}

int common_rmdir(filesystem_t *fs, const char *path) {
  Panic("not ready!");
}

int common_link(filesystem_t *fs, const char *path, inode_t *inode) {
  inode_t *pp = fs->ops->lookup(fs, path, O_RDWR);
  if (strlen(pp->path) == strlen(path)) {
    return -1;
  }

  inode_t *ip = pmm->alloc(sizeof(inode_t));
  ip->refcnt = 0;
  ip->type = TYPE_LINK;
  ip->ptr = (void *)inode;
  sprintf(ip->path, path);
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

int common_unlink(filesystem_t *fs, const char *path) {
  inode_t *ip = fs->ops->lookup(fs, path, O_RDWR);
  if (ip->type != TYPE_FILE || ip->type != TYPE_LINK) {
    return -1;
  }
  inode_delete(ip);
  return 0;
}

int common_readdir(filesystem_t *fs, inode_t *inode, char *ret) {
  sprintf(ret, "ls %s:\n", inode->path);
  strcat(ret, " + TYPE PRIV FILENAME\n");
  for (inode_t *ip = inode->fchild; ip != NULL; ip = ip->cousin) {
    CLog(FG_PURPLE, "%s %s", inode_types_human[ip->type], ip->path);
    strcat(ret, " - ");
    strcat(ret, inode_types_human[ip->type]);
    strcat(ret, " ");

    if (ip->type == TYPE_DIRC || ip->type == TYPE_MNTP) {
      strcat(ret, "D ");
    } else if (ip->type == TYPE_LINK) {
      strcat(ret, "L ");
    } else {
      strcat(ret, "- ");
    }
    strcat(ret, ip->flags & P_RD ? "R" : "-");
    strcat(ret, ip->flags & P_WR ? "W" : "-");
    strcat(ret, " ");

    strcat(ret, ip->path);
    strcat(ret, "\n");
  }
  return 0;
}

void mount_commonfs() {
  //vfs->mount("/mnt", &commonfs);
  device_t *dev = dev_lookup("ramdisk0");
  commonfs_init(&commonfs, "/", dev);
  CLog(BG_YELLOW, "/ initialized.");
}

void commonfs_init(filesystem_t *fs, const char *path, device_t *dev) {
  fs->dev = dev;
  fs->root->ptr = pmm->alloc(sizeof(commonfs_params_t));
  dev->ops->read(dev, 0, fs->root->ptr, sizeof(commonfs_params_t));
  int32_t blk = 1;
  while (blk) {
    commonfs_entry_t entry = commonfs_get_entry(fs, blk);  
    if (entry.type != TYPE_INVL) {
      inode_t *ip = pmm->alloc(sizeof(inode_t));
      ip->refcnt = 0;
      ip->type = (int)entry.type;
      ip->flags = (int)entry.flags;
      ip->ptr = (void *)(entry.head);
      sprintf(ip->path, "%s", path);
      if (path[strlen(path) - 1] == '/') {
        ip->path[strlen(path) - 1] = '\0';
      }
      strcat(ip->path, entry.path);
      ip->offset = 0;
      ip->size = commonfs_get_file_size(fs, &entry);
      ip->fs = fs;
      ip->ops = pmm->alloc(sizeof(inodeops_t));
      memcpy(ip->ops, &common_ops, sizeof(inodeops_t));

      inode_t *pp = inode_search(fs->root, ip->path);
      ip->parent = pp;
      ip->fchild = NULL;
      ip->cousin = NULL;
      inode_insert(pp, ip);
    }
    blk = commonfs_get_next_blk(fs, blk);
  }
}

inode_t *commonfs_lookup(filesystem_t *fs, const char *path, int flags) {
  inode_t *ip = inode_search(fs->root, path);
  return (strlen(ip->path) == strlen(path)) ? ip : NULL;
}

int commonfs_close(inode_t *inode) {
  if (inode->size <= 0) {
    inode_remove(inode->parent, inode);
  }
  return 0;
}
