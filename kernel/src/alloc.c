#include <common.h>
#include <klib.h>
#include <slab.h>
#include <spinlock.h>

struct spinlock kmm_lock;

static void pmm_init() {
  Log("HEAP AREA: [%p, %p)", _heap.start, _heap.end);
  spinlock_init(&kmm_lock, "KMM SPIN LOCK");
  kmem_init(_heap.start, _heap.end);
  Log("INIT PASSED.");
}

static void *kalloc(size_t size) {
  spinlock_acquire(&kmm_lock);
  struct kmem_cache *cp = kmem_cache_create(size);
  void *ret = kmem_cache_alloc(cp);
  Assert(ret, "MALLOC RETURNED NULL");
  Assert(ret >= _heap.start && ret <= _heap.end, "MALLOC NOT IN HEAP AREA");
  Assert(((struct kmem_item *) (ret - sizeof(struct kmem_item)))->used, "item is not marked as used!!");
  spinlock_release(&kmm_lock);
  return ret;
}

static void kfree(void *ptr) {
  spinlock_acquire(&kmm_lock);
  kmem_cache_free(ptr);
  spinlock_release(&kmm_lock);
}

MODULE_DEF(pmm) {
  .init = pmm_init,
  .alloc = kalloc,
  .free = kfree,
};
