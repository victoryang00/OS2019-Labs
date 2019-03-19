#ifndef __SLAB_H__
#define __SLAB_H__

#define Byte           sizeof(uint8_t)
#define KB             1024 * Byte
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

void *get_free_pages(int);
void free_used_pages(void *, int);

static inline void kmem_slab_chain_add(struct kmem_slab *head, struct kmem_slab *slab) {
  slab->next = NULL;
  if (head) {
    struct kmem_slab *sp = head;
    while (sp->next) sp = sp->next;
    sp->next = slab;
  } else {
    head = slab;
  }
};

static inline void kmem_slab_chain_remove(struct kmem_slab *head, struct kmem_slab *slab) {
  if (head == slab) {
    head = slab->next;
  } else {
    struct kmem_slab *sp = head;
    bool success = false;
    for ( ; sp && sp->next; sp = sp->next) {
      if (sp->next == slab) {
        sp->next = slab->next;
        success = true;
        break;
      }
    }
    assert(likely(success));
  }
  slab->next = NULL;
}

static inline void kmem_cache_move_slab_to_full(struct kmem_cache *cp, struct kmem_slab *sp) {
  slab_chain_remove(cp->slabs_free, sp);
  slab_chain_add(cp->slabs_full, sp);
}

static inline void kmem_cache_move_slab_to_free(struct kmem_cache *cp, struct kmem_slab *sp) {
  kmem_slab_chain_remove(cp->slabs_full, sp);
  kmem_slab_chain_add(cp->slabs_free, sp);
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
  sp->nr_items++;
  if (sp->nr_items >= sp->nr_items_max) {
    move_slab_to_full(sp);
  }
}

static inline void kmem_slab_remove_item(struct kmem_slab *sp, struct kmem_item *item) {
  if (sp->items == item) {
    sp->items = sp->items->next;
  } else {
    struct kmem_item *ip = sp->items;
    bool success = false;
    for ( ; ip && ip->next; ip = ip->next) {
      if (ip->next == item) {
        ip->next= item->next;
        success = true;
        break;
      }
    }
    assert(likely(success));
  }
  item->next = NULL;
  if (sp->nr_items >= sp->nr_items_max) {
    move_slab_to_free(sp);
  }
  sp->nr_items--;
}

#endif
