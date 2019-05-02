#include <common.h>
#include <os.h>
#include <spinlock.h>

struct spinlock printf_lock __attribute__((used));
struct spinlock os_trap_lock;

static struct os_handler root_handler = {
  0, _EVENT_NULL, NULL, NULL
};

static void os_init_locks() {
  spinlock_init(&printf_lock, "Printf SPIN LOCK");
  spinlock_init(&os_trap_lock, "OS TRAP SPIN LOCK");
}

static void os_init() {
  os_init_locks();
  CLog(BG_GREEN, "locks ok");
  pmm->init();
  CLog(BG_GREEN, "pmm ok");
  kmt->init();
  CLog(BG_GREEN, "kmt ok")
  //_vme_init(pmm->alloc, pmm->free);
  CLog(BG_GREEN, "vme ok");
  dev->init();
  CLog(BG_GREEN, "dev ok");
  //create proc here
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
  _Context *ret = NULL;

  spinlock_acquire(&os_trap_lock);
  for (struct os_handler *hp = root_handler.next; hp != NULL; hp = hp->next) {
    if (hp->event == _EVENT_NULL || hp->event == ev.event) {
      _Context *next = hp->handler(ev, context);
      if (next) ret = next;
    }
  }
  spinlock_release(&os_trap_lock);
  return ret;
}

static void os_on_irq(int seq, int event, handler_t handler) {
  struct os_handler *oh = pmm->alloc(sizeof(struct os_handler));
  oh->seq = seq;
  oh->event = event;
  oh->handler = handler;
  oh->next = NULL;

  spinlock_acquire(&os_trap_lock);
  struct os_handler *hp = &root_handler;
  while (hp->next && hp->next->seq < seq) hp = hp->next;
  oh->next = hp->next;
  hp->next = oh;
  spinlock_release(&os_trap_lock);
}

MODULE_DEF(os) {
  .init   = os_init,
  .run    = os_run,
  .trap   = os_trap,
  .on_irq = os_on_irq,
};
