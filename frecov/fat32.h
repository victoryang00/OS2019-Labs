#ifndef __FAT32_H__
#define __FAT32_H__

#include <stdint.h>

struct __attribute__((__packed__)) MBR {
  // common BPB structure
  char    BS_jmpBoot[3];    // 0x00 - 0x02
  char    BS_OEMName[8];    // 0x03 - 0x0A
  int16_t BPB_BytePerSec;   // 0x0B - 0x0C
  int8_t  BPB_SecPerClus;   // 0x0D
  int16_t BPB_RsvdSecCnt;   // 0x0E - 0x0F
  int8_t  BPB_NumFATs;      // 0x10
  int16_t BPB_RootEntCnt;   // 0x11 - 0x12
  int16_t BPB_TotSec16;     // 0x13 - 0x14
  int8_t  BPB_Media;        // 0x15
  int16_t BPB_FATSz16;      // 0x16 - 0x17
  int16_t BPB_SecPerTrk;    // 0x18 - 0x19
  int16_t BPB_NumHeads;     // 0x1A - 0x1B
  int64_t BPB_HiddSec;      // 0x1C - 0x1F
  int64_t BPB_TotSec32;     // 0x20 - 0x23

  // extended BPB structure for FAT32
  int64_t BPB_FATSz32;      // 0x24 - 0x27
  int32_t BPB_ExtFlags;     // 0x28 - 0x29
  int32_t BPB_FSVer;        // 0x2A - 0x2B
  int64_t BPB_RootClus;     // 0x2C - 0x2F
  int32_t BPB_FSInfo;       // 0x30 - 0x31
  int32_t BPB_BkBootSec;    // 0x32 - 0x33
  char    BPB_Reserved[12]; // 0x34 - 0x3F
  int8_t  BS_DrvNum;        // 0x40
  int8_t  BS_Reserved1;     // 0x41
  int8_t  BS_BootSig;       // 0x42
  int64_t BS_VolID;         // 0x43 - 0x46
  char    BS_VolLab[11];    // 0x47 - 0x51
  char    BS_FilSysTyle[8]; // 0x52 - 0x59
  char    EMPTY[420];       // 0x5A
  int16_t SignatureWord;    // 0x1FE
};

void *fat_load(const char *);

#endif
