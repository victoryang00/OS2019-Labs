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

  Log("offset of OEM is 0x%x", (int) ((void *) &mbr->BS_OEMName - (void *) mbr));
  Log("offset of EXBPB is 0x%x", (int) ((void *) &mbr->BPB_FATSz32 - (void *) mbr));
  Log("offset of empty space is 0x%x", (int) ((void *) &mbr->EMPTY - (void *) mbr));
  Log("offset of signature is 0x%x", (int) ((void *) &mbr->SignatureWord - (void *) mbr));
  Assert(mbr->SignatureWord == 0x55AA, "bad signature: read 0x%x, expect 0x55AA", mbr->SignatureWord);
  return ret;
}
