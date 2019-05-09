#include "frecov.h"

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: frecov FILE\n");
    printf("Check your input and try again.\n");
    exit(EXIT_FAILURE);
  } else {
    void *fat = fat_load(argv[1]);
  }
  return 0;
}
