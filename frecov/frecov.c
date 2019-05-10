#include "frecov.h"

static struct Disk *disk;
static struct DataSeg fdt_list = {
  NULL, NULL, &fdt_list, &fdt_list
};

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: frecov FILE\n");
    printf("Check your input and try again.\n");
    exit(EXIT_FAILURE);
  } else {
    disk = disk_load_fat(argv[1]);
    Log("image loaded at [%p, %p]", disk->head, disk->tail);
    recover_images();
  }
  return 0;
}

void recover_images() {
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
}

const char empty_entry[32] = {};
int get_cluster_type(void *c, int nr) {
  if (!memcmp(c, empty_entry, 32)) return TYPE_EMP;

  struct FDT *f = (struct FDT *) c;
  int fdt_count = 0;
  unsigned char chk_sum = 0;
  for (int i = 0; i < nr; ++i) {
    if (!f[i].file_size) continue;    // dir entry
    if (f[i].state == 0xe5) continue; // deleted
    if (!f[i].attr) return TYPE_BMP;  // bad: no attr
    if (f[i].attr == ATTR_LONG_NAME) {
      if (f[i].fst_clus) return TYPE_BMP; // bad: clus not 0
      if (f[i].type) return TYPE_BMP;     // bad: type not 0
      if (!fdt_count) {
        if (i && !(f[i].order & LAST_LONG_ENTRY)) return TYPE_BMP;
        fdt_count = f[i].order & ATTR_LONG_NAME;
        chk_sum = f[i].chk_sum;
      } else {
        if (f[i].chk_sum != chk_sum) return TYPE_BMP;
        if (f[i].order != --fdt_count) return TYPE_BMP;
      }
    } else {
      if (--fdt_count) return TYPE_BMP;
      if (chk_sum != check_sum((unsigned char *) f[i].name)) return TYPE_BMP;
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
    CLog(FG_BLUE, "fdt found at offset %x", (int) (c - disk->head));
    struct DataSeg *d = malloc(sizeof(struct DataSeg));
    d->head = c;
    d->tail = NULL;
    d->next = fdt_list.next;
    d->prev = &fdt_list;
    fdt_list.next = d;
    d->next->prev = d;
  }

  bool succ = true;
  while (succ) {
    succ = false;
    for (struct DataSeg *d = fdt_list.next; d != &fdt_list; d = d->next) {
      if (force) pos = 127;
      if (handle_fdt_aux(d->head, nr)) {
        CLog(FG_GREEN, "fdt at %x is handled!", (int) (d->head - disk->head));
        d->prev->next = d->next;
        d->next->prev = d->prev;
        free(d);
        succ = true;
      }
    }
  }
}
bool handle_fdt_aux(void *c, int nr) {
  struct FDT *f = (struct FDT *) c;
  if (pos == 127) {
    file_name[pos] = '\0';
  } else {
    if (f[0].attr == ATTR_LONG_NAME && f[0].chk_sum != chk_sum) {
      return false;
    }
  }

  for (int i = 0; i < nr; ++i) {
    if (f[i].attr == ATTR_LONG_NAME) {
      if (pos == 128) chk_sum = f[i].chk_sum;
      copy_name(f + i);
    } else {
      if (f[i].file_size) {
        uint32_t clus = ((uint32_t) f[i].fst_clus_HI << 16) | f[i].fst_clus_LO;
        printf("%x -> ", (int) ((void *) (f + i) - disk->head));
        printf("%s, ", file_name + pos);
        printf("clus = %u\n", clus);
      }
      pos = 127;
      file_name[pos] = '\0';
    }
  }
  return true;
}

void handle_bmp(void *p) {
  // TODO
}
