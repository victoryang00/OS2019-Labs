#include <common.h>
#include <klib.h>
#include <debug.h>

static void os_init() {
  pmm->init();
}

static void hello() {
  for (const char *ptr = "Hello from CPU #"; *ptr; ptr++) {
    _putc(*ptr);
  }
  _putc("012345678"[_cpu()]); _putc('\n');
}

static void kmem_test() {
  int *a = pmm->alloc(sizeof(int) * 4);
  printf("a[] is an 4-int array at %p", a);
  for (int i = 0; i < 4; ++i) {
    a[i] = i;
    printf("a[%d] is %d", i, a[i]);
  }
  pmm->free(a);
  printf("a is freed now!");
}

static void os_run() {
  hello();
  test_kmem();
  _intr_write(1);
  while (1) {
    _yield();
  }
}

static _Context *os_trap(_Event ev, _Context *context) {
  return context;
}

static void os_on_irq(int seq, int event, handler_t handler) {
}

MODULE_DEF(os) {
  .init   = os_init,
  .run    = os_run,
  .trap   = os_trap,
  .on_irq = os_on_irq,
};
