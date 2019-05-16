#include "frecov.h"

static struct Disk *disk;
static struct DataSeg fdt_list;
static struct DataSeg bmp_list[16][16][16];
static struct Image image_list;

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: frecov FILE\n");
    printf("Check your input and try again.\n");
    exit(EXIT_FAILURE);
  } else {
    disk = disk_load_fat(argv[1]);
    Log("image loaded at [%p, %p]", disk->head, disk->tail);

    struct stat st = {};
    if (stat(FOLDER, &st) == -1) {
      mkdir(FOLDER, 0700);
    }
    Assert(stat(FOLDER, &st) != -1, "create recovery folder failed.");

    recover_images();
  }
  return 0;
}

void recover_images() {
  fdt_list.prev = &fdt_list;
  fdt_list.next = &fdt_list;
  for (int i = 0; i < 16; ++i) {
    for (int j = 0; j < 16; ++j) {
      for (int k = 0; k < 16; ++k) {
        bmp_list[i][j][k].prev = &bmp_list[i][j][k];
        bmp_list[i][j][k].next = &bmp_list[i][j][k];
      }
    }
  }
  image_list.prev = &image_list;
  image_list.next = &image_list;

  size_t clusz = (size_t) disk->mbr->BPB_BytsPerSec * disk->mbr->BPB_SecPerClus;
  int nr_clu = clusz / 32;

  for (void *p = disk->data; p < disk->tail; p += clusz) {
    switch (get_cluster_type(p, nr_clu)) {
      case TYPE_FDT:
        handle_fdt(p, nr_clu, false);
      case TYPE_BMP:
        handle_bmp(p);
        break;
      default:
        break;
    }
  }
  handle_fdt(NULL, nr_clu, false);
  handle_fdt(NULL, nr_clu, true);

  for (struct Image *image = image_list.next; image != &image_list; image = image->next) {
    handle_image(image, clusz);
  }
}

const char empty_entry[32] = {};
int get_cluster_type(void *c, int nr) {
  if (!memcmp(c, empty_entry, 32)) return TYPE_EMP;

  struct FDT *f = (struct FDT *) c;
  bool has_long_name = false;
  unsigned char chk_sum = 0;
  for (int i = 0; i < nr; ++i) {
    if (i && !f[i].state && !f[i].attr) break;
    if (f[i].state == 0xe5) continue;
    if (f[i].attr == ATTR_DIRECTORY && !f[i].file_size) continue;
    if (f[i].attr == ATTR_LONG_NAME) {
      if (f[i].fst_clus) return TYPE_BMP;
      if (f[i].type) return TYPE_BMP;
      if (!has_long_name) {
        chk_sum = f[i].chk_sum;
        has_long_name = true;
      } else {
        if (f[i].chk_sum != chk_sum) return TYPE_BMP;
      }
    } else {
      if (__builtin_popcount(f[i].attr) != 1) return TYPE_BMP;
      if (has_long_name) {
        unsigned char cs = check_sum((unsigned char *) f[i].name);
        if (chk_sum != cs) return TYPE_BMP;
      }
      has_long_name = false;
    }
  }
  return TYPE_FDT;
}

static int pos = 127;
static char file_name[128] = {};
static unsigned char chk_sum = 0;
static inline void copy_name(struct FDT *f) {
  file_name[--pos] = f->name3[2];
  file_name[--pos] = f->name3[0];
  for (int i = 10; i >= 0; i -= 2) {
    file_name[--pos] = f->name2[i];
  }
  for (int i = 8; i >= 0; i -= 2) {
    file_name[--pos] = f->name1[i];
  }
}
void handle_fdt(void *c, int nr, bool force) {
  if (c) {
    //CLog(FG_BLUE, "fdt found at offset %x", (int) (c - disk->head));
    struct DataSeg *d = malloc(sizeof(struct DataSeg));
    d->head = c;
    d->next = fdt_list.next;
    d->prev = &fdt_list;
    fdt_list.next = d;
    d->next->prev = d;
  }

  bool succ = true;
  while (succ) {
    succ = false;
    for (struct DataSeg *d = fdt_list.next; d != &fdt_list; d = d->next) {
      if (handle_fdt_aux(d->head, nr, force)) {
        //CLog(FG_GREEN, "fdt at %x is handled!", (int) (d->head - disk->head));
        d->prev->next = d->next;
        d->next->prev = d->prev;
        free(d);
        succ = true;
        break;
      }
    }
  }
}
bool handle_fdt_aux(void *c, int nr, bool force) {
  struct FDT *f = (struct FDT *) c;
  if (force) {
    pos = 127;
  } else {
    //if (f[0].state == 0xe5) return false;
    if (f[0].attr == ATTR_LONG_NAME) {
      if (f[0].state & LAST_LONG_ENTRY) {
        chk_sum = f[0].chk_sum;
      } else {
        if (chk_sum != f[0].chk_sum) return false;
      }
    } else {
      if (f[0].attr != ATTR_DIRECTORY && pos != 127) {
        unsigned char cs = check_sum((unsigned char *) f[0].name);
        if (chk_sum != cs) return false;
      }
    }
  }

  for (int i = 0; i < nr; ++i) {
    if (f[i].attr == ATTR_LONG_NAME) {
      if (pos == 127) chk_sum = f[i].chk_sum;
      copy_name(f + i);
    } else {
      if (f[i].attr != ATTR_DIRECTORY && pos != 127) {
        size_t len = strlen(file_name + pos);
        if (!strncmp(file_name + pos + len - 4, ".bmp", 4)) {
          uint32_t clus = ((uint32_t) f[i].fst_clus_HI << 16) | f[i].fst_clus_LO;
          if (clus) {
            CLog(FG_GREEN, "%x -> %s, clus = %u", (int) ((void *) (f + i) - disk->head), file_name + pos, clus);
            struct Image *image = malloc(sizeof(struct Image));
            sprintf(image->name, FOLDER "/%s", file_name + pos);
            image->size = f[i].file_size;
            image->clus = clus;

            image->next = image_list.next;
            image->prev = &image_list;
            image_list.next = image;
            image->next->prev = image;
          }
        }
      }
      pos = 127;
      chk_sum = 0;
      file_name[pos] = '\0';
    }
  }
  return true;
}

void handle_bmp(void *p) {
  if (!strncmp((char *)p, "BM", 2)) return;
  Log("bmp seg at offset %x", (int) (p - disk->head));
  int i = ((int8_t *)p)[0] >> 4;
  int j = ((int8_t *)p)[1] >> 4;
  int k = ((int8_t *)p)[2] >> 4;

  struct DataSeg *d = malloc(sizeof(struct DataSeg));
  d->head = p;
  d->next = bmp_list[i][j][k].next;
  d->prev = &bmp_list[i][j][k];
  bmp_list[i][j][k].next = d;
  d->next->prev = d;
}

void handle_image(struct Image *image, size_t sz) {
  image->file = fopen(image->name, "w+");
  Assert(image->file, "fopen failed for image %s", image->name);

  void *clus = disk->data + sz * (image->clus - disk->mbr->BPB_RootClus);
  fwrite(image->file, sz, 1, clus);

  // be careful: size_t is unsigned!
  while (image->size > sz) {
    struct DataSeg *next = NULL;
    int32_t best_diff = 300; // maximum threshold

    int8_t *rgb_last = ((int8_t *) (clus + sz)) - 3;
    int i = rgb_last[0] >> 4;
    int j = rgb_last[1] >> 4;
    int k = rgb_last[2] >> 4;
    for (struct DataSeg *dp = bmp_list[i][j][k].next; dp != &bmp_list[i][j][k]; dp = dp->next) {
      int8_t *rgb_next = (int8_t *) dp->head;
      int32_t diff = 0;
      for (int i = 0; i < 3; ++i) {
        diff += (rgb_last[i] - rgb_next[i]) * (rgb_last[i] - rgb_next[i]);
      }
      if (diff < best_diff) {
        best_diff = diff;
        next = dp;
      }
    }

    if (!next) break;
    clus = next->head;
    next->prev->next = next->next;
    next->next->prev = next->prev;
    free(next);

    image->size -= sz;
    fwrite(image->file, sz, 1, clus);
  }
  fclose(image->file);

  if (image->size <= sz) {
    CLog(FG_YELLOW, "Image %s ready", image->name);
  } else {
    CLog(FG_RED, "Image %s failed", image->name);
  }
}
