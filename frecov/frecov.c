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
        break;
      case TYPE_BMP:
        handle_bmp(p, clusz);
        break;
      default:
        break;
    }
  }
  handle_fdt(NULL, nr_clu, false);
  handle_fdt(NULL, nr_clu, true);

  for (struct Image *image = image_list.next; image != &image_list; image = image->next) {
    handle_image(image, clusz, nr_clu);
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
    d->eof = false;
    d->holder = NULL;
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

            image->file = NULL;
            image->bmp  = malloc(sizeof(struct BMP));

            image->prev = image_list.prev;
            image->next = &image_list;
            image_list.prev = image;
            image->prev->next = image;
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

void handle_bmp(void *p, size_t sz) {
  if (!strncmp((char *)p, "BM", 2)) return;
  uint8_t i = ((uint8_t *)p)[0] >> 4;
  uint8_t j = ((uint8_t *)p)[1] >> 4;
  uint8_t k = ((uint8_t *)p)[2] >> 4;

  struct DataSeg *d = malloc(sizeof(struct DataSeg));
  d->head = p;
  d->eof  = true;
  d->holder = NULL;
  for (int i = 0; i < 4; ++i) {
    if ((((uint8_t *)(p + sz) - 4))[i] != 0) {
      d->eof = false;
      break;
    }
  }
  d->next = bmp_list[i][j][k].next;
  d->prev = &bmp_list[i][j][k];
  bmp_list[i][j][k].next = d;
  d->next->prev = d;
}

static inline uint32_t rgb_diff(uint8_t *a, uint8_t *b) {
  uint32_t ret = 0;
  for (int i = 0; i < 3; ++i) {
    // caution: a, b are unsigned.
    uint8_t d = a[i] > b[i] ? a[i] - b[i] : b[i] - a[i];
    ret += (uint32_t) d * (uint32_t) d;
  }
  return ret;
}
void handle_image(struct Image *image, size_t sz, int nr) {
  void *clus = disk->data + sz * (image->clus - disk->mbr->BPB_RootClus);
  struct BMP_Header *header = (struct BMP_Header *) clus;
  struct BMP_Info *info = (struct BMP_Info *) (header + 1);
  if (image->size != header->size || image->size < 32) {
    CLog(FG_RED, "bad file size, should be %d, get %d", (int)image->size, (int)header->size);
    return;
  }

  CLog(FG_GREEN, ">>>    start processing image %s", image->name);
  void *data = malloc(image->size);
  Assert(data, "malloc failed");

  size_t offset = (size_t) header->offset;
  size_t w = (size_t) (((24 * info->width + 31) >> 5) << 2);

  void *ptr = data;
  memcpy(ptr, clus, sz);
  ptr += sz;
  clus += sz;

  uint8_t *head = (uint8_t *) (head + offset);
  size_t delta = (sz - offset) / 24;
  size_t x = delta % w;
  size_t y = delta / w;

  size_t cnt = (image->size - 1) / sz;
  for (size_t t = 0; t < cnt; ++t) {
    uint8_t *rgb_down = y == 0 ? NULL : head + (y - 1) * w + x * 3;
    uint8_t *rgb_left = x == 0 ? NULL : head + y * w + (x - 1) * 3;
    
    bool sequent_ok = false;
    if (get_cluster_type(clus, nr) == TYPE_BMP) {
      uint32_t diff_down = rgb_down ? rgb_diff(rgb_down, (uint8_t *)clus) : 0;
      uint32_t diff_left = rgb_left ? rgb_diff(rgb_left, (uint8_t *)clus) : 0;
      if (!strcmp(image->name, "./recov/fuli.bmp")) {
        Log("diffs are %d %d", (int)diff_down, (int)diff_left);
      }
      sequent_ok = diff_down <= 300 && diff_left <= 300;
    }

    if (!sequent_ok) {
      CLog(FG_YELLOW, "sequent data segment at %x fails", (int) (clus - disk->head));
      clus = NULL;
      uint32_t best_diff_down = 30000; // maximum threshold
      uint32_t best_diff_left = 30000;
      struct DataSeg *next = NULL;

      uint8_t il = 0x0, jl = 0x0, kl = 0x0;
      uint8_t ir = 0xf, jr = 0xf, kr = 0xf;
      for (uint8_t i = il; i <= ir; ++i) {
        for (uint8_t j = jl; j <= jr; ++j) {
          for (uint8_t k = kl; k <= kr; ++k) {
            for (struct DataSeg *dp = bmp_list[i][j][k].next; dp != &bmp_list[i][j][k]; dp = dp->next) {
              if (dp->holder == image) continue;
              if ((t == cnt - 1) ^ dp->eof) continue;

              uint32_t diff_down = rgb_down ? rgb_diff(rgb_down, (uint8_t *)dp->head) : 0;
              uint32_t diff_left = rgb_left ? rgb_diff(rgb_left, (uint8_t *)dp->head) : 0;
              if (diff_down <= best_diff_down && diff_left <= best_diff_left) {
                best_diff_down = diff_down;
                best_diff_left = diff_left;
                next = dp;
              }
            }
          }
        }
      }

      clus = next->head;
      next->holder = image;
    }

    if (!clus) {
      CLog(FG_RED, "image %s failed", image->name);
    } else {
      if (t == cnt - 1) {
        memcpy(ptr, clus, image->size - cnt * sz);
      } else {
        memcpy(ptr, clus, sz);
        ptr += sz;
        clus += sz;
        x = (x + sz) % w;
        y = y + sz / w;
      }
    }
  }
  
#ifdef DEBUG
  image->file = fopen(image->name, "w+");
  Assert(image->file, "fopen failed for image %s", image->name);
  fwrite(data, image->size, 1, image->file);
  fclose(image->file);
#endif

  free(data);
  CLog(FG_GREEN, "<<< finished processing image %s", image->name);
}

