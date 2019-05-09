#include "frecov.h"

static struct Disk *disk;

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: frecov FILE\n");
    printf("Check your input and try again.\n");
    exit(EXIT_FAILURE);
  } else {
    disk = fat_load(argv[1]);
    Log("image loaded at %p", disk->head);
  }
  return 0;
}
