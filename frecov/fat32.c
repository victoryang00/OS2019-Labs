#include "frecov.h"

void *fat_load(const char *file) {
  int fd = open(file, O_RDONLY);
  Assert(fd, "failed to open file");

  struct stat sb;
  Assert(!fstat(fd, &sb), "fstat failed");
  Log("FILE SIZE IS %lu", sb.st_size);

  void *ret = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  Assert(ret != MAP_FAILED, "mmap failed");

  struct MBR *mbr = (struct MBR *) ret;
  Log("offset of signature is %lx", (void *) &mbr->signature - (void *) mbr);
  Assert(mbr->signature == 0xaa55, "bad MBR signature %x, should be 0xaa55", mbr->signature);
  return ret;
}
