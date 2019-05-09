#include "fat32.h"

void *fat_load(const char *file) {
  int fd = find(file, O_RDONLY);
  Assert(fd, "failed to open file.");

  struct stat sb;
  Assert(!fstat(fd, &sb), "fstat failed");
  Log("FILE SIZE IS %lu", (uint64_t) sb.st_size);

  void *ret = mmap(NULL, 0, PROT_READ, MAP_PRIVATE, fd, 0);
  Assert(ret != MAP_FAILED, "mmap failed");
  return ret;
}
