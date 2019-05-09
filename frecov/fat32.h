#ifndef __FAT32_H__
#define __FAT32_H__

#include <stdint.h>

#pragma pack(1)
struct MBR {
  // common BPB structure
  char     BS_jmpBoot[3];    // 0x00 - 0x02
  char     BS_OEMName[8];    // 0x03 - 0x0A
  uint16_t BPB_BytePerSec;   // 0x0B - 0x0C
  int8_t  BPB_SecPerClus;   // 0x0D
  uint16_t BPB_RsvdSecCnt;   // 0x0E - 0x0F
  uint8_t  BPB_NumFATs;      // 0x10
  uint16_t BPB_RootEntCnt;   // 0x11 - 0x12
  uint16_t BPB_TotSec16;     // 0x13 - 0x14
  uint8_t  BPB_Media;        // 0x15
  uint16_t BPB_FATSz16;      // 0x16 - 0x17
  uint16_t BPB_SecPerTrk;    // 0x18 - 0x19
  uint16_t BPB_NumHeads;     // 0x1A - 0x1B
  uint64_t BPB_HiddSec;      // 0x1C - 0x1F
  uint64_t BPB_TotSec32;     // 0x20 - 0x23

  // extended BPB structure for FAT32
  uint64_t BPB_FATSz32;      // 0x24 - 0x27
  uint32_t BPB_ExtFlags;     // 0x28 - 0x29
  uint32_t BPB_FSVer;        // 0x2A - 0x2B
  uint64_t BPB_RootClus;     // 0x2C - 0x2F
  uint32_t BPB_FSInfo;       // 0x30 - 0x31
  uint32_t BPB_BkBootSec;    // 0x32 - 0x33
  char     BPB_Reserved[12]; // 0x34 - 0x3F
  uint8_t  BS_DrvNum;        // 0x40
  uint8_t  BS_Reserved1;     // 0x41
  uint8_t  BS_BootSig;       // 0x42
  uint64_t BS_VolID;         // 0x43 - 0x46
  char     BS_VolLab[11];    // 0x47 - 0x51
  char     BS_FilSysTyle[8]; // 0x52 - 0x59
  char     EMPTY[420];       // 0x5A
  uint16_t SignatureWord;    // 0x1FE
};
#pragma pack()

void *fat_load(const char *);

#endif
