#include <common.h>
#include <slab.h>

void *slab_cache_ptr = NULL;

void slab_init(void *heap_end) {
  slab_cache_ptr = heap_end;
}

struct slab_cache *slab_cache_create(size_t size) {
  slab_cache_ptr -= sizeof(struct slab_cache);
  struct slab_cache *cp = (struct slab_cache *)(slab_cache_ptr);
  cp->size = size;
  cp->prev = NULL;
  cp->next = NULL;
  cp->slab_full = NULL;
  cp->slab_part = NULL;
  cp->slab_free = NULL;
  return cp;
}

void *slab_cache_alloc(struct slab_cache *) {

}

void slab_cache_grow() {

}

void slab_cache_free() {

}

void slab_cache_destroy() {

}

void slab_cache_reap() {

}
