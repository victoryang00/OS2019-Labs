#include "fat32.h"

void *fat_load(const char *file) {
  int fd = open(file, O_RDONLY);
  Assert(fd, "failed to open file, errno %d", errno);

  struct stat sb;
  Assert(!fstat(fd, &sb), "fstat failed, errno %d", errno);
  Log("FILE SIZE IS " FMT_LU, (uint64_t) sb.st_size);

  void *ret = mmap(NULL, 0, PROT_READ, MAP_PRIVATE, fd, 0);
  Assert(ret != MAP_FAILED, "mmap failed, errno %d", errno);
  return ret;
}
