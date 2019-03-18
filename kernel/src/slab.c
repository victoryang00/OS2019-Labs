#include <common.h>
#include <slab.h>

struct _slab_cache_head _master;
struct _slab_cache_head *slab_master = &_master;

void slab_init(void *heap_start, void *heap_end) {
  assert(heap_end > heap_start);
  slab_master->nr_pages = (heap_end - heap_start) / (SZ_PAGE + sizeof(bool));
  slab_master->page_memory = heap_start;
  slab_master->page_indicators = heap_start + nr_pages * SZ_PAGE;
  memset(slab_master->page_indicators, 0, nr_pages * sizeof(bool));
}

void* get_free_page() {
  for (int i = 0; i < slab_master->nr_pages; ++i) {
    if (*page_indicator(i)) {
      return page_translate(i);
    }
  }
  Log("NO FREE PAGES!");
  assert(0);
}

void free_used_page(void *pg_base) {
  int nr_pg = (pg_base - slab_master->page_memory) / SZ_PAGE;
  *page_indicator(nr_pg) = false;
}
