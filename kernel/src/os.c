#include <common.h>
#include <klib.h>
#include <spinlock.h>

struct spinlock printf_lock __attribute__((used));
struct spinlock os_trap_lock;

static void os_init_locks() {
  spinlock_init(&printf_lock, "Printf SPIN LOCK");
  spinlock_init(&os_trap_lock, "OS TRAP SPIN LOCK");
}

static void os_init() {
  //TODO: implement the following:
  os_init_locks();
  pmm->init();
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
  spinlock_acquire(&os_trap_lock);
  // TODO: what to do here??
  spinlock_release(&os_trap_lock);
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
