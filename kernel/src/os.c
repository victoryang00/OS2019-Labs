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
  int *a = pmm->alloc(sizeof(int) * 100);
  for (int i = 0; i < 100; ++i) {
    a[i] = i;
  }
  bool *b = pmm->alloc(sizeof(bool) * 100);
  for (int i = 0; i < 100; ++i) {
    b[i] = i;
  }
  char *c = pmm->alloc(sizeof(char) * 100);
  for (int i = 0; i < 100; ++i) {
    c[i] = i;
  }
  pmm->free(a);
  pmm->free(c);
  int *d = pmm->alloc(sizeof(double) * 2000);
  for (int i = 0; i < 2000; ++i) {
    d[i] = i * 1.14514;
  }
  pmm->free(d);
  pmm->free(b);
}

static void os_run() {
  hello();
  kmem_test();
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
