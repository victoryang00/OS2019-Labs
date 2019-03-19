#ifndef __SLAB_H__
#define __SLAB_H__

#define Byte           sizeof(uint8_t)
#define KB             1024 * Byte
#define SZ_PAGE        4 * KB
#define SZ_SMALL_OBJ   SZ_PAGE / 8
#define NR_CACHE_PAGES 2
#define NR_LARGE_ITEMS 4


#endif
