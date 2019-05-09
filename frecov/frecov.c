#include "frecov.h"

static struct Disk *disk;

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: frecov FILE\n");
    printf("Check your input and try again.\n");
    exit(EXIT_FAILURE);
  } else {
    disk = disk_load_fat(argv[1]);
    Log("image loaded at %p", disk->head);
    recover_images(disk);
  }
  return 0;
}

void recover_images(struct Disk *disk) {

}
