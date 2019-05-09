#ifndef __FAT32_H__
#define __FAT32_H__

struct MBR {
  char jmp_instr[3];      // 00
  char oem_name[8];       // 03
  char useless;           // 0A
  uint16_t byts_per_sec;  // 0B
  uint8_t sec_per_clus;   // 0D
  uint16_t rsvd_sec_cnt;  // 0E
  uint8_t num_fats;       // 10
  char useless2[0x13];    // 11
  uint32_t fat_sz_32;     // 24
  char useless3[0x04];    // 28
  uint32_t root_clus;     // 2C
  char useless4[0x18f];   // XX

  struct __attribute__((packed)) {
    char useless[16];
  } partition[4];

  uint16_t signature;
} __attribute__((packed));

void *fat_load(const char *);

#endif
