#include <common.h>
#include <klib.h>
#include <debug.h>

static void os_init() {
  pmm->init();
  //TODO: implement the following:
  //pmm->init();
  //kmt->init();
  //_vme_init(pmm->allow, pmm->free);
  //dev->init();
  ////create proc here
}

static void hello() {
  for (const char *ptr = "Hello from CPU #"; *ptr; ptr++) {
    _putc(*ptr);
  }
  _putc("012345678"[_cpu()]); _putc('\n');
}

static void os_run() {
  hello();
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
