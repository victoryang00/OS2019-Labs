#include "frecov.h"

static struct Disk *disk;
//static struct DataSeg *data_list;

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: frecov FILE\n");
    printf("Check your input and try again.\n");
    exit(EXIT_FAILURE);
  } else {
    disk = disk_load_fat(argv[1]);
    Log("image loaded at [%p, %p]", disk->head, disk->tail);
    recover_images(disk);
  }
  return 0;
}

void recover_images(struct Disk *disk) {
  size_t clusz = (size_t) disk->mbr->BPB_BytsPerSec * disk->mbr->BPB_SecPerClus;
  int nr_clu = clusz / 32;

  for (void *p = disk->data; p < disk->tail; p += clusz) {
    if (cluster_is_fdt(p, nr_clu)) {
      Log("%p -> fdt", p);
      handle_fdt(p, nr_clu);
    } else {
      Log("%p -> bmp", p);
      handle_bmp(p);
    }
  }
}

int fdt_count = 0;
unsigned char chksum = 0;
bool cluster_is_fdt(void *c, int nr) {
  struct FDT *f = (struct FDT *) c;
  for (int i = 0; i < nr; ++i) {
    if (!f[i].type) return false;
    if (f[i].type == ATTR_LONG_NAME) {
      if (f[i].fst_clus) return false;
      if (!fdt_count) {
        if (!(f[i].order & LAST_LONG_ENTRY)) return false;
        fdt_count = f[i].order & ATTR_LONG_NAME;
        chksum = f[i].chk_sum;
      } else {
        if (f[i].chk_sum != chksum) return false;
        if (f[i].order != --fdt_count) return false;
      }
    } else {
      if (fdt_count != 0) return false;
      Log("checksums: %u == %u", chksum, check_sum((unsigned char *) f[i].name));
      if (chksum != check_sum((unsigned char *) f[i].name)) return false;
    }
  }
  return true;
}

void handle_bmp(void *p) {
  // TODO
}

static int pos = 128;
static char file_name[128] = "";
static inline void move_name(struct FDT *f) {
  file_name[--pos] = f->name3[0];
  for (int i = 10; i >= 0; i -= 2) {
    file_name[--pos] = f->name2[i];
  }
  for (int i = 8; i >= 0; i -= 2) {
    file_name[--pos] = f->name1[i];
  }
}
void handle_fdt(void *c, int nr) {
  struct FDT *f = (struct FDT *) c;
  for (int i = 0; i < nr; ++i) {
    if (f[i].type == ATTR_LONG_NAME) {
      move_name(f + i);
    } else {
      printf("%s\n", file_name + pos);
      pos = 128;
      file_name[pos] = '\0';
    }
  }
}
