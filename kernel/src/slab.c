#include <common.h>
#include <slab.h>

int nr_pages = 0;
void *pm = NULL; // paging memory
bool *pi = NULL; // paging indicators
struct kmem_cache *kc = NULL; // kmem caches

void kmem_init(void *heap_start, void *heap_end) {
  assert(heap_end > heap_start);
  nr_pages = (heap_end - heap_start) / (SZ_PAGE + sizeof(bool)) - NR_CACHE_PAGES;
  pm = heap_start;
  kc = (struct kmem_cache *) (heap_start + nr_pages * SZ_PAGE);
  pi = (bool *) (kc + NR_CACHE_PAGES * SZ_PAGE);
  Log("pm=%p, kc=%p, pi=%p", pm, kc, pi);
  memset(kc, 0, NR_CACHE_PAGES * SZ_PAGE);
  memset(pi, 0, nr_pages * sizeof(bool));
}

struct kmem_cache* kmem_cache_create(size_t size) {
  struct kmem_cache *cp = kc;
  while (cp->item_size > 0 && cp->item_size != size) cp++;
  if (cp->item_size > 0 && cp->item_size == size) {
    Log("Cache of size %d exists.", size);
  } else {
    Log("Cache does not exist, create a new one.");
    Assert((void *) kc < (void *) pi, "Kcache zone is full.");
    cp = (struct kmem_cache *) (kc++);
  
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
  }
  return cp;
}

void kmem_cache_grow(struct kmem_cache *cp) {
  void *pg_start = get_free_pages(cp->nr_pages_alloc);
  Assert(pg_start != NULL, "No free pages of length %d in memory", cp->nr_pages_alloc);
  struct kmem_slab *sp = pg_start + cp->nr_pages_alloc * SZ_PAGE - sizeof(struct kmem_slab);
  
  sp->item_size = cp->item_size;
  sp->nr_items = 0;
  sp->nr_items_max = cp->nr_items_slab;
  sp->pg_start = pg_start;
  sp->nr_pages = cp->nr_pages_alloc;
  sp->items = NULL;
  sp->cache = cp;

  struct kmem_item *ip = pg_start;
  for (int i = 0; i < sp->nr_items_max; ++i) {
    ip->used = false;
    ip->slab = sp;
    kmem_slab_add_item(sp, ip);
    ip++;
  }
}

void *kmem_cache_alloc(struct kmem_cache *cp) {
  if (likely(cp->slabs_free == NULL)) {
    Log("No free slabs, allocating a new slab.");
    kmem_cache_grow(cp);
  }
  struct kmem_slab *sp = cp->slabs_free;
  struct kmem_item *ip = sp->items;
  while (ip && ip->used) ip = ip->next;
  Assert(ip, "Item pointer is null.");
  ip->used = true;
  sp->nr_items++;
  if (sp->nr_items >= sp->nr_items_max) {
    kmem_cache_move_slab_to_full(sp->cache, sp);
  }
  Log("item address %p, real address %p", ip, (void *)ip + sizeof(struct kmem_item));
  return ((void *) ip) + sizeof(struct kmem_item);
}

void kmem_cache_free(void *ptr) {
  struct kmem_item *ip = (struct kmem_item *) (ptr - sizeof(struct kmem_item));
  struct kmem_slab *sp = ip->slab;
  ip->used = false;
  if (sp->nr_items >= sp->nr_items_max) {
    kmem_cache_move_slab_to_free(sp->cache, sp);
  }
  sp->nr_items--;
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
  int b = (base - pm) / SZ_PAGE;
  for (int i = 0; i < nr; ++i) {
    *(pi + b + i) = false;
  }
}
