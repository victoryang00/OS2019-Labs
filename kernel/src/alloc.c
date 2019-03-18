#include <common.h>
#include <klib.h>
#include <slab.h>

static void pmm_init() {
  slab_init(_heap.start, _heap.end);
  Log("INIT PASSED.");
}

static void *kalloc(size_t size) {
  return NULL;
}

static void kfree(void *ptr) {
}

MODULE_DEF(pmm) {
  .init = pmm_init,
  .alloc = kalloc,
  .free = kfree,
};
