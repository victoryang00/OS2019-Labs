#include "frecov.h"

struct Disk *disk_load_fat(const char *file) {
  int fd = 0;
  struct stat sb = {};
  struct Disk *ret = malloc(sizeof(struct Disk));

  fd = open(file, O_RDONLY);
  Assert(fd, "failed to open file");
  Assert(!fstat(fd, &sb), "fstat failed");

  ret->head = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  ret->tail = ret->head + (size_t) sb.st_size;
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
  size_t fatsz = (size_t) disk->mbr->BPB_BytsPerSec * disk->mbr->BPB_FATSz32;
  disk->fat[0] = (struct FAT *) (((void *) disk->head) + offst);
  disk->fat[1] = (struct FAT *) (((void *) disk->head) + offst + fatsz * (disk->mbr->BPB_NumFATs - 1));
  Log("FAT1   at %p, offset 0x%x", disk->fat[0], (int) ((void *) disk->fat[0] - disk->head));
  Log("FAT2   at %p, offset 0x%x", disk->fat[1], (int) ((void *) disk->fat[1] - disk->head));

  offst += (size_t) disk->mbr->BPB_BytsPerSec * disk->mbr->BPB_NumFATs;
  offst += (size_t) disk->mbr->BPB_BytsPerSec * (disk->mbr->BPB_RootClus - 2) * disk->mbr->BPB_SecPerClus;
  disk->data = (((void *) disk->head) + offst);
  Log("DATA   at %p, offset 0x%x", disk->data, (int) ((void *) disk->data - disk->head));
}

unsigned char check_sum(unsigned char *c) {
  unsigned char sum = 0;
  for (short len = 11; len >= 0; --len) {
    sum = ((sum & 1) ? 0x80 : 0) + (sum >> 1) + *(c++);
  }
  return sum;
}
