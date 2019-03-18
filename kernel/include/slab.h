#ifndef __SLAB_H__
#define __SLAB_H__

#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#define SZ_PAGE       ((size_t) sysconf(_SC_PAGESIZE))
#define SZ_SMALL_OBJ  (SZ_PAGE / 8)
#define SZ_CACHE_LINE 0x40
#define SLAB_ALIGN    0x8

struct slab_buf;
struct slab_chain;
struct slab_cache;

struct slab_buf {
  void *buf;
  struct slab_buf *prev;
  struct slab_buf *next;
  struct slab_chain *slab;
}

struct slab_chain {
  int bufcount;
  void *free_list;
  struct slab_buf *start;
  struct slab_chain *prev;
  struct slab_chain *next;
}

struct slab_cache {
  size_t size;
  size_t effsize;
  int slab_maxbuf;
  struct slab_chain *prev;
  struct slab_chain *next;
}

struct slab_cache *slab_cache_create(char *, size_t, int);
void *slab_cache_alloc(struct slab_cache *);
void slab_cache_grow(struct slab_cache *);
void slab_cache_free(struct slab_cache *);
void slab_cache_destroy(struct slab_cache *);

#endif
