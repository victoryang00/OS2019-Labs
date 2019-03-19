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

static void test_kmem();

static void os_run() {
  hello();
  test_kmem();
  _intr_write(1);
  while (1) {
    _yield();
  }
}

static void test_kmem() {
  Log("TEST START!");
  int *a = pmm->alloc(sizeof(int));
  Log("Address of a is %p", a);
  *a = 1;
  Log("Value of a is %d", *a);
  pmm->free(a);
  Log("TEST PASS!");
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
