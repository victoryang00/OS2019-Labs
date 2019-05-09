#include "frecov.h"

struct Disk *disk_load_fat(const char *file) {
  struct Disk *ret = malloc(sizeof(struct Disk));

  int fd = open(file, O_RDONLY);
  Assert(fd, "failed to open file");

  struct stat sb;
  Assert(!fstat(fd, &sb), "fstat failed");

  ret->head = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  Assert(ret->head = MAP_FAILED, "mmap failed");

  struct MBR *mbr = (struct MBR *) ret;
  Assert(mbr->SignatureWord == 0xAA55, "bad signature: read 0x%x, expect 0xAA55", mbr->SignatureWord);

  disk_get_sections(ret);
  return ret;
}

void disk_get_sections(struct Disk *disk) {
  disk->mbr = (struct MBR *) disk;

  size_t offst = (size_t) disk->mbr->BPB_BytsPerSec * disk->mbr->BPB_RsvdSecCnt;
  size_t fatsz = (size_t) disk->mbr->BPB_BytsPerSec * disk->mbr->BPB_RsvdSecCnt;
  disk->fat[1] = (struct FAT **) (((void *) disk) + offst);
  disk->fat[2] = (struct FAT **) (((void *) disk->fat[1]) + fatsz);

  disk->fdt = (struct FDT **) (((void *) disk->fat[1]) + fatsz * disk->mbr->BPB_NumFATs);
}
