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
  //CLog(BG_GREEN, "vme ok");
  CLog(BG_RED, "vme not enabled!!!!!!");
  dev->init();
  CLog(BG_GREEN, "dev ok");
  //create proc here
  kmt->create(pmm->alloc(sizeof(task_t)), "Input Task", input_task, NULL);
  kmt->create(pmm->alloc(sizeof(task_t)), "TTY Task", tty_task, NULL);
}

static void hello() {
  //for (const char *ptr = "Hello from CPU #"; *ptr; ptr++) {
  //  _putc(*ptr);
  //}
  //_putc("012345678"[_cpu()]); _putc('\n');
  printf("Hello from CPU #%d\n", _cpu());
}

static void os_run() {
  hello();
  Log("HELLO OK!");
  _intr_write(1);
  Log("_intr_write(1) ok");
  while (1) {
    Log("Going to yield!!");
    _yield();
    Log("Back from yield.");
  }
}

static _Context *os_trap(_Event ev, _Context *context) {
  spinlock_acquire(&os_trap_lock);
  CLog(BG_CYAN, "Event %d: %s", ev.event, ev.msg);
  _Context *ret = NULL;
  for (struct os_handler *hp = root_handler.next; hp != NULL; hp = hp->next) {
    if (hp->event == _EVENT_NULL || hp->event == ev.event) {
      _Context *next = hp->handler(ev, context);
      if (next) ret = next;
    }
  }
  spinlock_release(&os_trap_lock);

  Assert(ret != NULL, "Returning to a null context after trap.");
  return ret;
}

static void os_on_irq(int seq, int event, handler_t handler) {
  CLog(BG_PURPLE, "Handler for event class %d (seq=%d) added.", event, seq);
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
