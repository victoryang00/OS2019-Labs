#include "frecov.h"

static struct Disk *disk;
static struct DataSeg *data_list;

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
  struct FDT *fp = disk->fdt;
  size_t clusz = (size_t) disk->BPB_BytsPerSec * disk->BPB_SecPerClus;
  while ((void *) fp < disk->tail) {
    // TODO: ??
  }
}
