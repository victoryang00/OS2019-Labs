#include <common.h>
#include <slab.h>

struct _slab_cache_head _master;
struct _slab_cache_head *slab_master = &_master;

void slab_init(void *heap_start, void *heap_end) {
  assert(heap_end > heap_start);
  slab_master->nr_pages = (heap_end - heap_start) / (SZ_PAGE + sizeof(bool)) - NR_CACHE_PAGES;
  assert(slab_master->nr_pages > 0);
  slab_master->page_memory = heap_start;
  slab_master->page_indicators = (bool *) (heap_start + (slab_master->nr_pages + NR_CACHE_PAGES) * SZ_PAGE);
  slab_master->cache_memory = (struct _slab_cache *) (heap_start + slab_master->nr_pages * SZ_PAGE);
  memset(slab_master->page_indicators, 0, slab_master->nr_pages * sizeof(bool));
}

struct _slab_cache *slab_cache_create(size_t size) {
  struct _slab_cache *p_cache = slab_master->cache_memory++;
  assert((void *) slab_master->cache_memory < (void *) slab_master->page_indicators);
  p_cache->size = size;
  if (size <= SZ_SMALL_OBJ) {
    p_cache->items_per_chain = (SZ_PAGE - sizeof(struct _slab_chain)) / size;
  } else {
    p_cache->items_per_chain = NR_LARGE_ITEMS;
  }
  // nr_pages should be ceil of the result.
  p_cache->pages_per_chain = (p_cache->items_per_chain * size + sizeof(struct _slab_chain) - 1) / SZ_PAGE + 1;
  return p_cache;
}

void slab_cache_grow(struct _slab_cache *p_cache) {
  void *page = NULL;
  struct _slab_chain *p_chain;
  page = get_free_page(p_cache->pages_per_allocate);
  p_chain = page + p_cache->pages_per_allocate * SZ_PAGE - sizeof(struct _slab_chain);
  assert(p_chain); // NOT NULL
  p_chain->page = page;
  p_chain->next = p_cache;
  p_chain->prev = p_cache->prev;
  p_cache->prev->next = p_chain;
  p_cache->prev = p_chain;
  p_chain->parent = p_cache;
  return p_chain;
}
