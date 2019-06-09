#include <common.h>
#include <file.h>
#include <vfs.h>

typedef struct naivefs_params {
  int32_t blk_size;
  int32_t map_head;
  int32_t data_head;
  int32_t min_free;
} naivefs_params_t;

typedef union naivefs_entry {
  char content[32];
  struct {
    int32_t head;
    int16_t type;
    int16_t flags;
    char path[24];
  };
} naivefs_entry_t;

int32_t naivefs_get_next_blk(filesystem_t *fs, int32_t blk) {
  int32_t ret = 0;
  naivefs_params_t *params = (naivefs_params_t *)fs->root->ptr;
  off_t offset = params->map_head + blk * sizeof(int32_t);
  fs->dev->ops->read(fs->dev, offset, (void *)(&ret), sizeof(int32_t));
  return ret;
}

int32_t naivefs_get_last_entry_blk(filesystem_t *fs) {
  int32_t blk = 1;
  while (true) {
    int32_t next = naivefs_get_next_blk(fs, blk);
    if (next == 0) return blk;
    else blk = next;
  }
}

void naivefs_put_params(filesystem_t *fs, naivefs_params_t *params) {
  fs->dev->ops->write(fs->dev, 0, (void *)params, sizeof(naivefs_params_t));
}

void naivefs_add_map(filesystem_t *fs, int32_t from, int32_t to) {
  naivefs_params_t *params = (naivefs_params_t *)fs->root->ptr;
  fs->dev->ops->write(fs->dev, params->map_head + from * sizeof(int32_t), (void *)(&to), sizeof(int32_t));
}

naivefs_entry_t naivefs_get_entry(filesystem_t *fs, int32_t blk) {
  naivefs_entry_t ret;
  naivefs_params_t *params = (naivefs_params_t *)fs->root->ptr;
  off_t offset = params->data_head + blk * params->blk_size;
  fs->dev->ops->read(fs->dev, offset, (void *)(&ret), sizeof(naivefs_entry_t));
  return ret;
}

void naivefs_put_entry(filesystem_t *fs, int32_t blk, naivefs_entry_t *entry) {
  naivefs_params_t *params = (naivefs_params_t *)fs->root->ptr;
  off_t offset = params->data_head + blk * params->blk_size;
  fs->dev->ops->write(fs->dev, offset, (void *)entry, sizeof(naivefs_entry_t));
}

void naivefs_add_entry(filesystem_t *fs, naivefs_entry_t *entry) {
  naivefs_params_t *params = (naivefs_params_t *)fs->root->ptr;
  Log("2-1");
  int32_t last = naivefs_get_last_entry_blk(fs);
  int32_t blk = params->min_free;
  ++params->min_free;
  Log("2-2");
  naivefs_add_map(fs, last, blk);
  Log("2-3");
  naivefs_put_entry(fs, blk, entry);
  Log("2-4");
  naivefs_put_params(fs, params);
  Log("2-5");
}

size_t naivefs_get_file_size(filesystem_t *fs, naivefs_entry_t *entry) {
  size_t ret = 0;
  int32_t blk = entry->head;
  naivefs_params_t *params = (naivefs_params_t *)fs->root->ptr;
  while (true) {
    int32_t next = naivefs_get_next_blk(fs, blk);
    if (next) {
      ret += params->blk_size;
    } else {
      naivefs_entry_t ce = naivefs_get_entry(fs, blk);
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

inodeops_t naive_ops = {
  .open    = naive_open,
  .close   = naive_close,
  .read    = naive_read,
  .write   = naive_write,
  .lseek   = naive_lseek,
  .mkdir   = naive_mkdir,
  .rmdir   = naive_rmdir,
  .link    = naive_link,
  .unlink  = naive_unlink,
  .readdir = naive_readdir,
};

fsops_t naivefs_ops = {
  .init   = naivefs_init,
  .lookup = naivefs_lookup,
  .close  = naivefs_close,
};

filesystem_t naivefs = {
  .name = "naivefs",
  .root = NULL,
  .ops  = &naivefs_ops,
  .dev  = NULL,
};

int naive_open(filesystem_t *fs, file_t *file, int flags) {
  return 0;
}

int naive_close(filesystem_t *fs, file_t *file) {
  return 0;
}

ssize_t naive_read(filesystem_t *fs, file_t *file, char *buf, size_t size) {
  naivefs_params_t *params = (naivefs_params_t *)fs->root->ptr;
  off_t offset = file->inode->offset;
  int32_t blk = (int32_t)file->inode->ptr;
  
  while (offset >= params->blk_size) {
    offset -= params->blk_size;
    blk = naivefs_get_next_blk(fs, blk);
    if (blk == 0) return 0;
  }

  ssize_t nread = 0;
  while (blk != 0 && size > 0) {
    naivefs_entry_t entry = naivefs_get_entry(fs, blk);
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
    blk = naivefs_get_next_blk(fs, blk);
  }
  return nread;
}

ssize_t naive_write(filesystem_t *fs, file_t *file, const char *buf, size_t size) {
  Panic("not ready!");
}

off_t naive_lseek(filesystem_t *fs, file_t *file, off_t offset, int whence) {
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

int naive_mkdir(filesystem_t *fs, const char *path) {
  inode_t *pp = inode_search(fs->root, path);
  if (strlen(pp->path) == strlen(path)) return -1;
  Log("1");

  naivefs_entry_t entry = {
    .head = 0x00000000,
    .type = TYPE_DIRC,
    .flags = P_RD | P_WR,
  };
  Log("2");
  snprintf(entry.path, 24, path);
  naivefs_add_entry(fs, &entry);
  Log("3");
  
  inode_t *ip = pmm->alloc(sizeof(inode_t));
  ip->refcnt = 0;
  ip->type = TYPE_DIRC;
  ip->flags = P_RD | P_WR;
  sprintf(ip->path, path);
  Log("4");
  ip->offset = 0;
  ip->size = 4;
  ip->fs = fs;
  ip->ops = pmm->alloc(sizeof(inodeops_t));
  memcpy(ip->ops, &naive_ops, sizeof(inodeops_t));
  Log("5");

  ip->parent = pp;
  ip->fchild = NULL;
  ip->cousin = NULL;
  inode_insert(pp, ip);
  Log("6");
  return 0;
}

int naive_rmdir(filesystem_t *fs, const char *path) {
  Panic("not ready!");
}

int naive_link(filesystem_t *fs, const char *path, inode_t *inode) {
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
  memcpy(ip->ops, &naive_ops, sizeof(inodeops_t));

  ip->parent = pp;
  ip->fchild = NULL;
  ip->cousin = NULL;
  inode_insert(pp, ip);
  return 0;
}

int naive_unlink(filesystem_t *fs, const char *path) {
  inode_t *ip = fs->ops->lookup(fs, path, O_RDWR);
  if (ip->type != TYPE_FILE || ip->type != TYPE_LINK) {
    return -1;
  }
  inode_delete(ip);
  return 0;
}

int naive_readdir(filesystem_t *fs, inode_t *inode, char *ret) {
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

void mount_naivefs() {
  //vfs->mount("/mnt", &naivefs);
  device_t *dev = dev_lookup("ramdisk0");
  naivefs_init(&naivefs, "/", dev);
  CLog(BG_YELLOW, "/ initialized.");
}

void naivefs_init(filesystem_t *fs, const char *path, device_t *dev) {
  fs->dev = dev;
  fs->root->ptr = pmm->alloc(sizeof(naivefs_params_t));
  dev->ops->read(dev, 0, fs->root->ptr, sizeof(naivefs_params_t));
  int32_t blk = 1;
  while (blk) {
    naivefs_entry_t entry = naivefs_get_entry(fs, blk);  
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
      ip->size = naivefs_get_file_size(fs, &entry);
      ip->fs = fs;
      ip->ops = pmm->alloc(sizeof(inodeops_t));
      memcpy(ip->ops, &naive_ops, sizeof(inodeops_t));

      inode_t *pp = inode_search(fs->root, ip->path);
      ip->parent = pp;
      ip->fchild = NULL;
      ip->cousin = NULL;
      inode_insert(pp, ip);
    }
    blk = naivefs_get_next_blk(fs, blk);
  }
}

inode_t *naivefs_lookup(filesystem_t *fs, const char *path, int flags) {
  inode_t *ip = inode_search(fs->root, path);
  return (strlen(ip->path) == strlen(path)) ? ip : NULL;
}

int naivefs_close(inode_t *inode) {
  if (inode->size <= 0) {
    inode_remove(inode->parent, inode);
  }
  return 0;
}
