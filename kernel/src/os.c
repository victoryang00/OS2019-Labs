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

static void* s[4][1145] = {};
static void test() {
  srand(uptime());
  for (int i = 0; i < 1145; ++i) {
    s[_cpu()][i] = pmm->alloc(rand() % (4096));
  }
  for (int i = 0; i < 1145; ++i) {
    pmm->free(s[_cpu()][i]);
  }
  CLog(BG_GREEN, "SUCCESS ON CPU %d", _cpu());
  printf("SUCCESS ON CPU %d", _cpu());
}

static void os_run() {
  hello();
  test();
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
