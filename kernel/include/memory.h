#ifndef __SLAB_H__
#define __SLAB_H__

#ifdef MEM_DEBUG
#define MEMLog(...)  Log(__VA_ARGS__)
#define MEMCLog(...) CLog(__VA_ARGS__)
#else
#define MEMLog(...) 
#define MEMCLog(...)
#endif

#define KB             1024
#define SZ_PAGE        4 * KB
#define SZ_SMALL_OBJ   SZ_PAGE / 8
#define NR_CACHE_PAGES 2
#define NR_LARGE_ITEMS 4

struct kmem_item;
struct kmem_slab;
struct kmem_cache;

struct kmem_item {
  bool used;
  struct kmem_item *next;
  struct kmem_slab *slab;
};

struct kmem_slab {
  size_t item_size;
  int nr_items;
  int nr_items_max;
  void *pg_start;
  int nr_pages;
  struct kmem_item *items;
  struct kmem_slab *next;
  struct kmem_cache *cache;
};

struct kmem_cache {
  size_t item_size;
  int nr_items_slab;
  int nr_pages_alloc;
  struct kmem_slab *slabs_free;
  struct kmem_slab *slabs_full;
};

void kmem_init(void *, void *);
struct kmem_cache* kmem_cache_create(size_t);
void kmem_cache_grow(struct kmem_cache *cp);
void *kmem_cache_alloc(struct kmem_cache *cp);
void kmem_cache_free(void *);

void *get_free_pages(int);
void free_used_pages(void *, int);

static inline void kmem_cache_add_slab(struct kmem_cache *cp, struct kmem_slab *slab) {
  slab->next = NULL;
  if (cp->slabs_free) {
    struct kmem_slab *sp = cp->slabs_free;
    while (sp->next) sp = sp->next;
    sp->next = slab;
  } else {
    cp->slabs_free = slab;
  }
};

static inline void kmem_cache_remove_slab(struct kmem_cache *cp, struct kmem_slab *slab) {
  if (cp->slabs_free == slab) {
    cp->slabs_free = slab->next;
  } else {
    struct kmem_slab *sp = cp->slabs_free;
    bool success = false;
    for ( ; sp && sp->next; sp = sp->next) {
      if (sp->next == slab) {
        sp->next = slab->next;
        success = true;
        break;
      }
    }
    Assert(likely(success), "Slab does not exist in chain!!");
  }
  slab->next = NULL;
}

static inline void kmem_cache_move_slab_to_full(struct kmem_cache *cp, struct kmem_slab *slab) {
  kmem_cache_remove_slab(cp, slab);
  slab->next = NULL;
  if (cp->slabs_full) {
    struct kmem_slab *sp = cp->slabs_full;
    while (sp->next) sp = sp->next;
    sp->next = slab;
  } else {
    cp->slabs_full = slab;
  }
  MEMLog("Slab of size %d at %p moved to full list.", slab->item_size, slab);
}

static inline void kmem_cache_move_slab_to_free(struct kmem_cache *cp, struct kmem_slab *slab) {
  if (cp->slabs_full == slab) {
    cp->slabs_full = slab->next;
  } else {
    struct kmem_slab *sp = cp->slabs_full;
    bool success = false;
    for ( ; sp && sp->next; sp = sp->next) {
      if (sp->next == slab) {
        sp->next = slab->next;
        success = true;
        break;
      }
    }
    Assert(likely(success), "Slab does not exist in chain!!");
  }
  kmem_cache_add_slab(cp, slab);
  MEMLog("Slab of size %d at %p moved to free list.", slab->item_size, slab);
}

static inline void kmem_slab_add_item(struct kmem_slab *sp, struct kmem_item *item) {
  item->next = NULL;
  if (sp->items) {
    struct kmem_item *ip = sp->items;
    while (ip->next) ip = ip->next;
    ip->next = item;
  } else {
    sp->items = item;
  }
}

#endif
