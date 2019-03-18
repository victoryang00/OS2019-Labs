#include <common.h>
#include <slab.h>

void *slab_cache_ptr = NULL;

void slab_init(void *heap_end) {
  slab_cache_ptr = heap_end;
}

struct slab_cache *slab_cache_create(size_t size) {
  slab_cache_ptr -= sizeof(struct slab_cache);
  struct slab_cache *scp = (struct slab_cache *)(slab_cache_ptr);
  scp->size = size;
  scp->prev = NULL;
  scp->next = NULL;
  scp->slab_full = NULL;
  scp->slab_part = NULL;
  scp->slab_free = NULL;
  return scp;
}

void *slab_cache_alloc(struct slab_cache *scp) {
  return NULL;
}

void slab_cache_grow(struct slab_cache *scp) {

}

void slab_cache_free(struct slab_cache *scp) {

}

void slab_cache_destroy(struct slab_cache *scp) {

}

void slab_cache_reap() {

}
