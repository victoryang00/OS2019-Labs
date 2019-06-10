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
  if (from == to) to = 0; // for reformatted disks (like 1 -> 1)
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

int32_t naivefs_add_entry(filesystem_t *fs, naivefs_entry_t *entry) {
  naivefs_params_t *params = (naivefs_params_t *)fs->root->ptr;
  int32_t last = naivefs_get_last_entry_blk(fs);
  int32_t blk = params->min_free;
  ++params->min_free;
  naivefs_add_map(fs, last, blk);
  naivefs_put_entry(fs, blk, entry);
  naivefs_put_params(fs, params);
  return blk;
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

filesystem_t emptyfs = {
  .name = "emptyfs",
  .root = NULL,
  .ops  = &naivefs_ops,
  .dev  = NULL,
};

int naive_open(filesystem_t *fs, file_t *file, int flags) {
  if ((flags & file->inode->flags) != (flags & ~O_CREAT)) return E_BADPR;
  file->inode->offset = 0;
  return 0;
}

int naive_close(filesystem_t *fs, file_t *file) {
  if (file->inode->offset > file->inode->size) {
    file->inode->size = file->inode->offset;
  }
  file->inode->offset = 0;
  return fs->ops->close(file->inode);
}

ssize_t naive_read(filesystem_t *fs, file_t *file, char *buf, size_t size) {
  naivefs_params_t *params = (naivefs_params_t *)fs->root->ptr;
  inode_t *ip = file->inode;
  while (ip->type == TYPE_LINK) ip = (inode_t *)ip->ptr;
  off_t offset = ip->offset;
  int32_t blk = ip->blk;
  
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
      delta = snprintf(buf + nread, size, "%s", entry.content + offset);
    } else {
      delta = snprintf(buf + nread, params->blk_size - offset, "%s", entry.content + offset);
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
  naivefs_params_t *params = (naivefs_params_t *)fs->root->ptr;
  inode_t *ip = file->inode;
  while (ip->type == TYPE_LINK) ip = (inode_t *)ip->ptr;
  off_t offset = ip->offset;
  int32_t blk = ip->blk;
  
  while (offset >= params->blk_size) {
    offset -= params->blk_size;
    blk = naivefs_get_next_blk(fs, blk);
    if (blk == 0) return 0;
  }

  ssize_t nwrite = 0;
  while (blk != 0 && size > 0) {
    naivefs_entry_t entry = naivefs_get_entry(fs, blk);
    ssize_t delta = 0;
    if (params->blk_size - offset >= size) {
      delta = snprintf(entry.content + offset, size, "%s", buf + nwrite);
    } else {
      delta = snprintf(entry.content + offset, params->blk_size - offset, "%s", buf + nwrite);
    }
    naivefs_put_entry(fs, blk, &entry);
    if (delta == 0) break;
    size -= delta;
    nwrite += delta;
    
    offset = 0;
    naivefs_add_map(fs, blk, params->min_free);
    blk = params->min_free;
    ++params->min_free;
  }
  
  ip->offset += nwrite;
  if (ip->offset > ip->size) {
    ip->size = ip->offset;
  }
  return nwrite;
}

off_t naive_lseek(filesystem_t *fs, file_t *file, off_t offset, int whence) {
  inode_t *ip = file->inode;
  while (ip->type == TYPE_LINK) ip = (inode_t *)ip->ptr;
  switch (whence) {
    case SEEK_SET:
      ip->offset = offset;
      break;
    case SEEK_CUR:
      ip->offset += offset;
      break;
    case SEEK_END:
    default:
      ip->offset = ip->size + offset;
      break;
  }
  return ip->offset;
}

int naive_mkdir(filesystem_t *fs, const char *path) {
  inode_t *pp = inode_search(fs->root, path);
  if (strlen(pp->path) == strlen(path)) return E_ALRDY;

  naivefs_entry_t entry = {
    .head = 0x00000000,
    .type = TYPE_DIRC,
    .flags = P_RD | P_WR,
  };
  if(strlen(path) - strlen(fs->root->path) >= 23) return E_TOOLG; // naivefs limitation
  sprintf(entry.path, "%s", path + strlen(fs->root->path));
  int32_t blk = naivefs_add_entry(fs, &entry);
  
  inode_t *ip = pmm->alloc(sizeof(inode_t));
  ip->refcnt = 0;
  ip->type = TYPE_DIRC;
  ip->flags = P_RD | P_WR;
  ip->blk = blk;
  sprintf(ip->path, "%s", path);
  ip->offset = 0;
  ip->size = 4;
  ip->fs = fs;
  ip->ops = pmm->alloc(sizeof(inodeops_t));
  memcpy(ip->ops, &naive_ops, sizeof(inodeops_t));

  ip->parent = pp;
  ip->fchild = NULL;
  ip->cousin = NULL;
  inode_insert(pp, ip);
  return 0;
}

int naive_rmdir(filesystem_t *fs, const char *path) {
  inode_t *ip = fs->ops->lookup(fs, path, O_RDWR);
  if (!ip) return E_NOENT;
  if (ip->type != TYPE_DIRC) return E_BADTP;
  if (!(ip->flags & O_WRONLY)) return E_BADPR;
  if (ip->fchild) return E_NOEMP;

  int32_t blk = ip->blk;
  naivefs_entry_t entry = naivefs_get_entry(fs, blk);
  entry.type = TYPE_INVL;
  naivefs_put_entry(fs, blk, &entry);

  inode_delete(ip);
  pmm->free(ip->ops);
  pmm->free(ip);
  return 0;
}

int naive_link(filesystem_t *fs, const char *path, inode_t *inode) {
  if (inode->type != TYPE_FILE && inode->type != TYPE_LINK) return E_BADTP;
  inode_t *pp = inode_search(root, path);
  if (strlen(pp->path) == strlen(path)) return E_ALRDY;

  naivefs_entry_t entry = {
    .head = inode->blk,
    .type = TYPE_LINK,
    .flags = P_RD | P_WR,
  };
  if(strlen(path) >= 23) return E_TOOLG; // naivefs limitation
  sprintf(entry.path, "%s", path);
  int32_t blk = naivefs_add_entry(fs, &entry);

  inode_t *ip = pmm->alloc(sizeof(inode_t));
  ip->refcnt = 0;
  ip->type = TYPE_LINK;
  ip->flags = P_RD | P_WR;
  ip->ptr = (void *)inode;
  ip->blk = blk;
  sprintf(ip->path, "%s", path);
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
  if (!ip) return E_NOENT;
  if (ip->type != TYPE_FILE && ip->type != TYPE_LINK) return E_BADTP;
  if (!(ip->flags & O_WRONLY)) return E_BADPR;

  int32_t blk = ip->blk;
  naivefs_entry_t entry = naivefs_get_entry(fs, blk);
  entry.type = TYPE_INVL;
  naivefs_put_entry(fs, blk, &entry);

  inode_delete(ip);
  pmm->free(ip->ops);
  pmm->free(ip);
  return 0;
}

int naive_readdir(filesystem_t *fs, inode_t *inode, char *ret) {
  sprintf(ret, "ls %s:\n", inode->path);
  strcat(ret, " + TYPE PRIV SIZE FILENAME\n");
  for (inode_t *ip = inode->fchild; ip != NULL; ip = ip->cousin) {
    VFSCLog(FG_PURPLE, "%s %s", inode_types_human[ip->type], ip->path);
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

    char size[8] = "";
    if (ip->type == TYPE_FILE) {
      snprintf(size, 4, "%04d", ip->size);
    } else if (ip->type == TYPE_LINK) {
      inode_t *rip = ip;
      while (rip->type == TYPE_LINK) rip = (inode_t *)rip->ptr;
      snprintf(size, 4, "%04d", rip->size);
    } else {
      snprintf(size, 4, "----");
    }
    strcat(ret, size);
    strcat(ret, " ");

    strcat(ret, ip->path);
    strcat(ret, "\n");
  }
  return 0;
}

void mount_naivefs() {
  device_t *dev = dev_lookup("ramdisk0");
  naivefs_init(&naivefs, "/", dev);
  VFSCLog(BG_YELLOW, "/ initialized.");
  
  dev = dev_lookup("ramdisk1");
  naivefs_init(&emptyfs, "/mnt", dev);
  VFSCLog(BG_YELLOW, "/mnt initialized.");
  vfs->mount("/mnt", &emptyfs);
}

void naivefs_init(filesystem_t *fs, const char *path, device_t *dev) {
  fs->dev = dev;

  if (!fs->root) {
    fs->root = pmm->alloc(sizeof(inode_t));
    fs->root->type = TYPE_MNTP;
    fs->root->flags = P_RD | P_WR;
    fs->root->fs = fs;
    fs->root->ptr = NULL;
    sprintf(fs->root->path, "%s", path);
    fs->root->parent = NULL;
    fs->root->fchild = NULL;
    fs->root->cousin = NULL;
    fs->root->ops = &naive_ops;
  }
  fs->root->ptr = pmm->alloc(sizeof(naivefs_params_t));
  dev->ops->read(dev, 0, fs->root->ptr, sizeof(naivefs_params_t));

  naivefs_params_t *params = (naivefs_params_t *)fs->root->ptr;
  if (params->blk_size == 0) {
    params->blk_size  = 0x00000020;
    params->map_head  = 0x00000010;
    params->data_head = 0x00002000;
    params->min_free  = 0x00000000;
    dev->ops->write(dev, 0, params, sizeof(naivefs_params_t));

    naivefs_entry_t entry = {
      .head = 0x00000000,
      .type = (int16_t)TYPE_MNTP,
      .flags = (int16_t)(P_RD | P_WR),
    };
    sprintf(entry.path, "/");
    naivefs_add_entry(fs, &entry);
  }

  int32_t blk = 1;
  while (blk) {
    naivefs_entry_t entry = naivefs_get_entry(fs, blk);  
      Log("blk %d", blk);
    if (entry.type != TYPE_INVL) {
      inode_t *ip = pmm->alloc(sizeof(inode_t));
      ip->refcnt = 0;
      ip->type = (int)entry.type;
      ip->flags = (int)entry.flags;
      ip->blk = entry.head;
      if (ip->type == TYPE_LINK) {
        Log("is a link!");
        naivefs_entry_t link_entry = naivefs_get_entry(fs, ip->blk);
        inode_t *lp = inode_search(fs->root, link_entry.path);
        if (strlen(lp->path) != strlen(link_entry.path)) continue;
        Log("link valid");
        ip->ptr = (void *)lp;
      }
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
  if (strlen(ip->path) == strlen(path)) {
    if ((flags & ip->flags) == (flags & ~O_CREAT)) {
      return ip;
    } else {
      return NULL;
    }
  } else {
    if (flags & O_CREAT) {
      VFSLog("not found. create a new one!");
      size_t len = strlen(path);
      if (len - strlen(fs->root->path) >= 23) return NULL; // naivefs limitation
      for (size_t i = strlen(ip->path) + 1; i < len; ++i) {
        if (path[i] == '/') return NULL;
      }

      naivefs_params_t *params = (naivefs_params_t *)fs->root->ptr;
      naivefs_entry_t entry = {
        .head = params->min_free,
        .type = TYPE_FILE,
        .flags = P_RD | P_WR,
      };
      sprintf(entry.path, "%s", path + strlen(fs->root->path));
      ++params->min_free;
      VFSLog("data will be in blk %d", entry.head);
      int32_t blk = naivefs_add_entry(fs, &entry);
      VFSLog("entry is put in blk %d", blk);

      inode_t *pp = ip;
      ip = pmm->alloc(sizeof(inode_t));
      ip->refcnt = 0;
      ip->type = TYPE_FILE;
      ip->flags = P_RD | P_WR;
      ip->blk = blk;
      sprintf(ip->path, "%s", path);
      ip->offset = 0;
      ip->size = 0;
      ip->fs = fs;
      ip->ops = pmm->alloc(sizeof(inodeops_t));
      memcpy(ip->ops, &naive_ops, sizeof(inodeops_t));

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

int naivefs_close(inode_t *inode) {
  if (inode->type == TYPE_FILE && inode->size <= 0) {
    int32_t blk = inode->blk;
    naivefs_entry_t entry = naivefs_get_entry(inode->fs, blk);
    entry.type = TYPE_INVL;
    naivefs_put_entry(inode->fs, blk, &entry);
    inode_remove(inode->parent, inode);
    pmm->free(inode->ops);
    pmm->free(inode);
  }
  return 0;
}
