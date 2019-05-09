#include "frecov.h"

struct Disk *disk_load_fat(const char *file) {
  int fd = 0;
  struct stat sb = {};
  struct Disk *ret = malloc(sizeof(struct Disk));

  fd = open(file, O_RDONLY);
  Assert(fd, "failed to open file");
  Assert(!fstat(fd, &sb), "fstat failed");

  ret->head = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  Assert(ret->head != MAP_FAILED, "mmap failed");

  disk_get_sections(ret);
  return ret;
}

void disk_get_sections(struct Disk *disk) {
  disk->mbr = (struct MBR *) disk->head;
  Assert(disk->mbr->SignatureWord == 0xaa55, "Expecting signature 0xaa55, got 0x%x", disk->mbr->SignatureWord);
  Log("MBR    at %p, offset 0x%x", disk->mbr, (int) ((void *) disk->mbr - disk->head));

  disk->fsinfo = (struct FSInfo *) ((void *) disk->mbr + 512);
  Log("FSInfo at %p, offset 0x%x", disk->fsinfo, (int) ((void *) disk->fsinfo - disk->head));

  size_t offst = (size_t) disk->mbr->BPB_BytsPerSec * disk->mbr->BPB_RsvdSecCnt;
  size_t fatsz = (size_t) disk->mbr->BPB_BytsPerSec * disk->mbr->BPB_RsvdSecCnt;
  disk->fat[1] = (struct FAT **) (((void *) disk) + offst);
  disk->fat[2] = (struct FAT **) (((void *) disk->fat[1]) + fatsz);
  Log("FAT1   at %p, offset 0x%x", disk->fat[1], (int) ((void *) disk->fat[1] - disk->head));
  Log("FAT2   at %p, offset 0x%x", disk->fat[2], (int) ((void *) disk->fat[2] - disk->head));

  disk->fdt = (struct FDT **) (((void *) disk->fat[1]) + fatsz * disk->mbr->BPB_NumFATs);
  Log("DATA   at %p, offset 0x%x", disk->fdt, (int) ((void *) disk->fdt - disk->head));
}

unsigned char check_sum(unsigned char *c) {
  unsigned char sum = 0;
  for (short len = 11; len >= 0; --len) {
    sum = ((sum & 1) ? 0x80 : 0) + (sum >> 1) + *(c++);
  }
  return sum;
}
