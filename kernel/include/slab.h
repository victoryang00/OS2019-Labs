#ifndef __SLAB_H__
#define __SLAB_H__

#define Byte sizeof(uint8_t)
#define KB 1024 * Byte
#define SZ_PAGE 4 * KB
#define NR_ITEMS_MAX 64

struct _slab_chain;
struct _slab_chain_head;
struct _slab_cache;
struct _slab_cache_head;

struct _slab_chain {
  void *page;
  struct _slab_chain *prev;
  struct _slab_chain *next;
  struct _slab_cache *parent;
};

struct _slab_chain_head {
  struct _slab_chain *prev;
  struct _slab_chain *next;
};

struct _slab_cache {
  size_t item_size;
  uint32_t items_per_page;
  struct _slab_chain_head slab_free;
  struct _slab_chain_head slab_full;
};

struct _slab_cache_head {
  int nr_pages;
  void *page_memory;
  bool *page_indicators;
  struct _slab_cache* prev;
  struct _slab_cache* next;
};
extern struct _slab_cache_head *slab_master;

void slab_init(void *, void *);
void *get_free_page();
void free_used_page(void *);

static inline void *page_translate(int n) {
  return slab_master->page_memory + n * SZ_PAGE;
}

static inline bool *page_indicator(int n) {
  return slab_master->page_indicators + n;
}

#endif
