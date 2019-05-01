#include <common.h>
#include <memory.h>

static int nr_pages = 0;
static void *pm = NULL; // paging memory
static bool *pi = NULL; // paging indicators
static struct kmem_cache *kc = NULL; // kmem caches

void kmem_init(void *heap_start, void *heap_end) {
  Assert(heap_end > heap_start, "INVALID HEAP SIZE!");
  nr_pages = (heap_end - heap_start) / (SZ_PAGE + sizeof(bool)) - NR_CACHE_PAGES;
  pm = heap_start;
  kc = (struct kmem_cache *) (heap_start + nr_pages * SZ_PAGE);
  pi = (bool *) (heap_start + (nr_pages + NR_CACHE_PAGES) * SZ_PAGE);
  Log("pm=%p, kc=%p, pi=%p.", pm, kc, pi);
  Assert((void *) pm >= heap_start && (void *) pm + nr_pages * SZ_PAGE <= heap_end,       "pm is invalid!");
  Assert((void *) kc >= heap_start && (void *) kc + NR_CACHE_PAGES * SZ_PAGE <= heap_end, "kc is invalid!");
  Assert((void *) pi >= heap_start && (void *) pi + nr_pages * sizeof(bool) <= heap_end,  "pi is invalid!");
  memset(kc, 0, NR_CACHE_PAGES * SZ_PAGE);
  memset(pi, 0, nr_pages * sizeof(bool));
}

struct kmem_cache* kmem_cache_create(size_t size) {
  struct kmem_cache *cp = kc;

  size = power2ify(size + sizeof(struct kmem_item)); // use power of 2 as size
  Log("Looking for cache of size %d (actually %d).", size, size - sizeof(struct kmem_item));
  while (cp->item_size > 0 && cp->item_size != size) {
    //Log("However, cache at %p has item size %d.", cp, cp->item_size);
    cp++;
  }
  if (cp->item_size > 0 && cp->item_size == size) {
    Log("Cache of size %d exists at %p.", size, cp);
  } else {
    Log("Cache of size %d does not exist, create a new one at %p.", cp->item_size, cp);
    Assert((void *) kc < (void *) pi, "Kcache zone is full.");
    cp->item_size = size;
    if (cp->item_size <= SZ_SMALL_OBJ) {
      cp->nr_items_slab = (SZ_PAGE - sizeof(struct kmem_slab)) / cp->item_size;
      cp->nr_pages_alloc = 1;
    } else {
      cp->nr_items_slab = NR_LARGE_ITEMS;
      cp->nr_pages_alloc = (cp->item_size * NR_LARGE_ITEMS + sizeof(struct kmem_slab) - 1) / (SZ_PAGE) + 1;
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
    Assert((void *) ip >= pm && (void *) ip < (void *) kc, "Item is outside of pm area!");
    ip->used = false;
    ip->slab = sp;
    kmem_slab_add_item(sp, ip);
    ip = (struct kmem_item *) ((void *) ip + sp->item_size);
  }

  kmem_cache_add_slab(cp, sp);
}

void *kmem_cache_alloc(struct kmem_cache *cp) {
  if (likely(cp->slabs_free == NULL)) {
    Log("No free slabs, allocating a new slab of %d items.", cp->nr_items_slab);
    kmem_cache_grow(cp);
    Assert(likely(cp->slabs_free != NULL), "Still no slab after growth.");
  } else {
    Log("The first free slab at %p has %d free items left.", cp->slabs_free, cp->nr_items_slab - cp->slabs_free->nr_items);
  }
  struct kmem_slab *sp = cp->slabs_free;
  struct kmem_item *ip = sp->items;
  while (likely(ip->used)) ip = ip->next;
  Assert((void *) ip >= pm && (void *) ip < (void *) kc, "Item %p is not in pm area [%p, %p)!", ip, pm, (void *) kc);
  ip->used = true;
  if (sp->nr_items < sp->nr_items_max) sp->nr_items++;
  if (sp->nr_items >= sp->nr_items_max) {
    kmem_cache_move_slab_to_full(sp->cache, sp);
  }
  Assert(likely(ip->used), "Item is not marked as used after allocation!!");
  CLog(BG_GREEN, "Memory allocated at %p, slab at %p has %d items free now.", (void *)ip + sizeof(struct kmem_item), sp, sp->nr_items_max - sp->nr_items);
  return ((void *) ip) + sizeof(struct kmem_item);
}

void kmem_cache_free(void *ptr) {
  struct kmem_item *ip = (struct kmem_item *) (ptr - sizeof(struct kmem_item));
  struct kmem_slab *sp = ip->slab;
  Assert(likely(ip->used), "Access Violation: Double freeing address kmem_item %p.", ip);
  ip->used = false;
  if (sp->nr_items >= sp->nr_items_max) {
    kmem_cache_move_slab_to_free(sp->cache, sp);
  }
  if (sp->nr_items > 0) sp->nr_items--;
  Assert(sp->nr_items >= 0, "Slab at %p has negative number of items!!", sp);
  CLog(BG_GREEN, "Item at %p freed. Slab at %p has %d items free now.", ptr, sp, sp->nr_items_max - sp->nr_items);
}

void* get_free_pages(int nr) {
  Log("Getting %d free memory pages.", nr);
  for (int i = 0; i < nr_pages - nr; ) {
    int j = 0;
    bool success = true;
    for (j = 0; j < nr; ++j) {
      if (likely(*(pi + i + j))) {
        success = false;
        break;
      }
    }
    if (likely(success)) {
      for (int j = 0; j < nr; ++j) {
        *(pi + i + j) = true;
      }
      Log("Memory pages start at %p.", pm + i * SZ_PAGE);
      return pm + i * SZ_PAGE;
    } else {
      i += j + 1;
    }
  }
  return NULL;
}

void free_used_pages(void *base, int nr) {
  Log("Freeing %d memory pages from %p.", nr, base);
  int b = (base - pm) / SZ_PAGE;
  for (int i = 0; i < nr; ++i) {
    *(pi + b + i) = false;
  }
}
