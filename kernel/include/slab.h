#ifndef __SLAB_H__
#define __SLAB_H__

#define Byte           sizeof(uint8_t)
#define KB             1024 * Byte
#define SZ_PAGE        4 * KB
#define SZ_SMALL_OBJ   SZ_PAGE / 8
#define NR_CACHE_PAGES 2
#define NR_LARGE_ITEMS 4

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
  int items_per_chain;
  int pages_per_chain;
  struct _slab_chain_head slab_free;
  struct _slab_chain_head slab_full;
};

struct _slab_cache_head {
  int nr_pages;
  void *page_memory;
  bool *page_indicators;
  struct _slab_cache* cache_memory;
  struct _slab_cache* prev;
  struct _slab_cache* next;
};
extern struct _slab_cache_head *slab_master;



void slab_init(void *, void *);
struct _slab_cache *slab_cache_create(size_t);
void slab_cache_grow(struct _slab_cache *);


static inline void *page_translate(int n) {
  return slab_master->page_memory + n * SZ_PAGE;
}

static inline bool *page_indicator(int n) {
  return slab_master->page_indicators + n;
}

static inline void *get_free_page(int n) {
  bool success;
  for (int i = 0; i < slab_master->nr_pages - n; ++i) {
    success = true;
    for (int j = 0; j < n; ++j) {
      if (likely(*page_indicator(i + j))) {
        success = false;
        break;
      }
    }
    if (success) {
      for (int j = 0; j < n; ++j) {
        *page_indicator(i + j) = true;
      }
      return page_translate(i);
    }
  }
  Log("NO FREE PAGES!");
  assert(0);
}

static inline void free_used_page(void *pg_base) {
  int nr_pg = (pg_base - slab_master->page_memory) / SZ_PAGE;
  *page_indicator(nr_pg) = false;
}

#endif
