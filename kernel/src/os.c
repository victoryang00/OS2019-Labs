#include <common.h>
#include <os.h>
#include <devices.h>
#include <threads.h>
#include <spinlock.h>
#include <semaphore.h>

struct spinlock printf_lock __attribute__((used));
struct spinlock os_trap_lock;

static struct os_handler root_handler = {
  0, _EVENT_NULL, NULL, NULL
};

sem_t sem_p;
sem_t sem_c;
sem_t mutex;
void customer(void *arg) {
  while (1) {
    kmt->sem_wait(&sem_c);
    kmt->sem_wait(&mutex);
    printf(")");
    CLog(BG_RED, ")");
    kmt->sem_signal(&mutex);
    kmt->sem_signal(&sem_p);
  }
}
void producer(void *arg) {
  while (1) {
    kmt->sem_wait(&sem_p);
    kmt->sem_wait(&mutex);
    printf("(");
    CLog(BG_RED, "(");
    kmt->sem_signal(&mutex);
    kmt->sem_signal(&sem_c);
  }
}

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
  CLog(BG_GREEN, "kmt ok");
  //_vme_init(pmm->alloc, pmm->free);
  //CLog(BG_GREEN, "vme ok");
  //CLog(BG_RED, "vme not enabled!!!!!!");
  dev->init();
  //CLog(BG_GREEN, "dev ok");

  //create proc here
  // FOR TEST PURPOSE
  // REMOVE WHEN TEST IS PASSED
  kmt->sem_init(&sem_p, "Producer SEM", 5);
  kmt->sem_init(&sem_c, "Customer SEM", 0);
  kmt->sem_init(&mutex, "Producer-Customer MUTEX", 1);
  for (int i = 0; i < 5; ++i) {
    kmt->create(pmm->alloc(sizeof(task_t)), "Producer Task", producer, NULL);
    kmt->create(pmm->alloc(sizeof(task_t)), "Customer Task", customer, NULL);
  }
}

static void os_run() {
  printf("Hello from CPU #%d\n", _cpu());
  _intr_write(1);
  Assert(_intr_read() != 0, "Interrupt disabled");
  while (1) {
    _yield();
  }
}

static _Context *os_trap(_Event ev, _Context *context) {
  if (ev.event == _EVENT_ERROR) {
    Panic("BAD EVENT %d: %s, caused by (%p of %p)", ev.event, ev.msg, ev.cause, ev.ref);
  }
  CLog(BG_CYAN, "Event %d: %s", ev.event, ev.msg);
  if (ev.event == _EVENT_IRQ_IODEV) printf("X");

  Assert(!spinlock_holding(&os_trap_lock), "trap in trap!");
  spinlock_acquire(&os_trap_lock);
  CLog(FG_CYAN, "Lock acquired. Begin trap process.");
  _Context *ret = NULL;
  for (struct os_handler *hp = root_handler.next; hp != NULL; hp = hp->next) {
    CLog(FG_CYAN, "Handler seq %d", hp->seq);
    if (hp->event == _EVENT_NULL || hp->event == ev.event) {
      _Context *next = hp->handler(ev, context);
      Assert(!next || (next->eip && next->esp0), "NULL EIP/ESP0 of the return context");
      Assert(!next || *((void **) (next->esp0)), "return context will return to 0x0000000");
      if (next) ret = next;
    }
  }
  CLog(FG_CYAN, "Lock released. Trap process finished.");
  spinlock_release(&os_trap_lock);

  Assert(ret != NULL, "Returning to a null context after trap.");
  //Log("Current context: %p", context);
  //Log("   Next context: %p", ret);
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
