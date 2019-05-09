#ifndef __FAT32_H__
#define __FAT32_H__

#include <stdint.h>

struct MBR {
  // common BPB structure
  char     BS_jmpBoot[3];    // 0x00 - 0x02
  char     BS_OEMName[8];    // 0x03 - 0x0A
  uint16_t BPB_BytsPerSec;   // 0x0B - 0x0C
  uint8_t  BPB_SecPerClus;   // 0x0D
  uint16_t BPB_RsvdSecCnt;   // 0x0E - 0x0F
  uint8_t  BPB_NumFATs;      // 0x10
  uint16_t BPB_RootEntCnt;   // 0x11 - 0x12
  uint16_t BPB_TotSec16;     // 0x13 - 0x14
  uint8_t  BPB_Media;        // 0x15
  uint16_t BPB_FATSz16;      // 0x16 - 0x17
  uint16_t BPB_SecPerTrk;    // 0x18 - 0x19
  uint16_t BPB_NumHeads;     // 0x1A - 0x1B
  uint32_t BPB_HiddSec;      // 0x1C - 0x1F
  uint32_t BPB_TotSec32;     // 0x20 - 0x23

  // extended BPB structure for FAT32
  uint32_t BPB_FATSz32;      // 0x24 - 0x27
  uint16_t BPB_ExtFlags;     // 0x28 - 0x29
  uint16_t BPB_FSVer;        // 0x2A - 0x2B
  uint32_t BPB_RootClus;     // 0x2C - 0x2F
  uint16_t BPB_FSInfo;       // 0x30 - 0x31
  uint16_t BPB_BkBootSec;    // 0x32 - 0x33
  char     BPB_Reserved[12]; // 0x34 - 0x3F
  uint8_t  BS_DrvNum;        // 0x40
  uint8_t  BS_Reserved1;     // 0x41
  uint8_t  BS_BootSig;       // 0x42
  uint32_t BS_VolID;         // 0x43 - 0x46
  char     BS_VolLab[11];    // 0x47 - 0x51
  char     BS_FilSysType[8]; // 0x52 - 0x59
  char     EMPTY[420];       // 0x5A
  uint16_t SignatureWord;    // 0x1FE
} __attribute__((packed));

struct FAT {
  union {
    uint8_t  mark[4];
    uint32_t next;
  };
} __attribute__((packed));

struct FSInfo {
  uint32_t FSI_LeadSig;        // 0x000
  char     FSI_Reserved1[480]; // 0x004
  uint32_t FSI_StrucSig;       // 0x1E4
  uint32_t FSI_Free_Count;     // 0x1E8
  uint32_t FSI_Nxt_Free;       // 0x1EC
  char     FSI_Reserved2[12];  // 0x1F0
  uint32_t FSI_TrailSig;       // 0x1FE
} __attribute__((packed));

#define ATTR_READ_ONLY 0x01
#define ATTR_HIDDEN    0x02
#define ATTR_SYSTEM    0x04
#define ATTR_VOLUME_ID 0x08
#define ATTR_DIRECTORY 0x10
#define ATTR_ARCHIVE   0x20

struct FDT {
  char     DIR_Name[11];     // 0x00 - 0x0A
  uint8_t  DIR_Attr;         // 0x0B
  uint8_t  DIR_NTRes;        // 0x0C
  uint8_t  DIR_CrtTimeTenth; // 0x0D
  uint16_t DIR_CrtTime;      // 0x0E - 0x0F
  uint16_t DIR_CrtDate;      // 0x10 - 0x11
  uint16_t DIR_LstAccDate;   // 0x12 - 0x13
  uint16_t DIR_FstClusHI;    // 0x14 - 0x15
  uint16_t DIR_WrtTime;      // 0x16 - 0x17
  uint16_t DIR_WrtDate;      // 0x18 - 0x19
  uint16_t DIR_FstClusLO;    // 0x1A - 0x1B
  uint32_t DIR_FileSize;     // 0x1C - 0x1F
} __attribute__((packed));

struct Disk {
  void *head;
  struct MBR *mbr;
  struct FAT **fat[2];
  struct FDT **fdt;
};

struct Disk *disk_load_fat(const char *);
void disk_get_sections(struct Disk *);

#endif
