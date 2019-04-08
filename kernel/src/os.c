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

static void *spaces[4][1005] = {};
static void test() {
  for (int i = 0; i < 1000; ++i) {
    spaces[_cpu()][i] = pmm->alloc(400);
  }
  for (int i = 0; i < 1000; ++i) {
    pmm->free(spaces[_cpu()][i]);
  }
  printf("SUCCESS ON CPU %d\n", _cpu());
}

static void os_run() {
  hello();
  _intr_write(1);
  test();
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
