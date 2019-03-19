#include <common.h>
#include <slab.h>

int nr_pages = 0;
void *pm = NULL; // paging memory
void *pi = NULL; // paging indicators
void *kc = NULL; // kmem caches

void kmem_init(void *heap_start, void *heap_end) {
  assert(heap_end > heap_start);
  nr_pages = (heap_end - heap_start) / (SZ_PAGE + sizeof(bool)) - NR_CACHE_PAGES;
  pm = heap_start;
  kc = heap_start + nr_pages * SZ_PAGE;
  pi = kc + NR_CACHE_PAGES * SZ_PAGE;
  memset(pi, 0, nr_pages * sizeof(bool));
}

struct kmem_cache* kmem_cache_create(size_t size) {
  if (kc >= pi) return NULL;
  struct kmem_cache* cp = (struct kmem_cache *)kc;
  kc += sizeof(kmem_cache);
  
  cp->item_size = sizeof(struct kmem_item) + size;
  if (cp->item_size <= SZ_SMALL_OBJ) {
    cp->nr_items_slab = (SZ_PAGE - sizeof(struct kmem_slab)) / cp->item_size;
    cp->nr_pages_alloc = 1;
  } else {
    cp->nr_items_slab = NR_LARGE_ITEMS;
    cp->nr_pages_alloc = (cp->item_size * NR_LARGE_ITEMS + sizeof(struct kmem_slab) - 1) / SZ_PAGE;
  }
  cp->slabs_free = NULL;
  cp->slabs_full = NULL;
  return cp;
}

void kmem_cache_grow(struct kmem_cache *cp) {
  void *pg_start = get_free_pages(cp->nr_pages_alloc);
  assert(pg_start != NULL);
  struct kmem_slab *sp = pg_start + cp->nr_pages_alloc * SZ_PAGE - sizeof(struct kmem_slab);
  
  sp->item_size = cp->item_size;
  sp->nr_items = 0;
  sp->nr_items_max = cp->nr_items_slab;
  sp->pg_start = pg_start;
  sp->nr_pages = cp->nr_pages_alloc;
  sp->items = NULL;
  sp->cache = cp;

  for (int i = 0; i < sp->nr_items_max; ++i) {
    
  }
}

void* get_free_pages(int nr) {
  bool success = true;
  for (int i = 0; i < nr_pages - nr; ++i) {
    success = true;
    for (int j = 0; j < nr; ++j) {
      if (likely(*(pi + i + j))) {
        success = false;
        break;
      }
    }
    if (likely(success)) {
      for (int j = 0; j < nr; ++j) {
        *(pi + i + j) = true;
      }
      return pm + i * SZ_PAGE;
    }
  }
  return NULL;
}

void free_used_pages(void *base, int nr) {
  int b = (base - pm_start) / SZ_PAGE;
  for (int i = 0; i < nr; ++i) {
    *(pi + b + i) = false;
  }
}
