#include <common.h>
#include <os.h>
#include <devices.h>
#include <threads.h>
#include <spinlock.h>
#include <semaphore.h>

struct spinlock os_trap_lock = {
  "OS Trap Lock", 0, -1
};
static struct os_handler root_handler = {
  0, _EVENT_NULL, NULL, NULL
};

static void os_init() {
  pmm->init();
  CLog(BG_GREEN, "pmm ok");

  kmt->init();
  CLog(BG_GREEN, "kmt ok");

  //_vme_init(pmm->alloc, pmm->free);
  //CLog(BG_GREEN, "vme ok");
  //CLog(BG_RED, "vme not enabled!!!!!!");
  
  dev->init();
  CLog(BG_GREEN, "dev ok");

  kmt->create(pmm->alloc(sizeof(task_t)), "shell", shell_task, 1);
}

static void os_run() {
  _intr_write(1);
  while (1) {
    // in order to save CPU,
    // do not _yield, just wait for timer.
    hlt();
  }
  Panic("os run cannot return");
}

static _Context *os_trap(_Event ev, _Context *context) {
  CLog(BG_CYAN, "Event %d: %s", ev.event, ev.msg);

  bool holding = spinlock_holding(&os_trap_lock);
  if (holding) {
    switch (ev.event) {
      case _EVENT_NULL:
        Panic("Null interrupt not allowed.\n");
        return context;
      case _EVENT_IRQ_TIMER:
        Panic("No timer interrupt during trap.\n");
        return context;
      case _EVENT_IRQ_IODEV:
        break;
      case _EVENT_YIELD:
        Panic("No yield inside trap.\n");
        return context;
      case _EVENT_ERROR:
        break;
      default:
        Panic("Other types of trap in trap.\n");
        return context;
    }

    for (struct os_handler *hp = root_handler.next; hp != NULL; hp = hp->next) {
      if (hp->event == ev.event) hp->handler(ev, context);
    }
    return context;
  } else {
    spinlock_acquire(&os_trap_lock);
    CLog(FG_PURPLE, ">>>>>> IN TO TRAP");
    _Context *ret = NULL;
    for (struct os_handler *hp = root_handler.next; hp != NULL; hp = hp->next) {
      Assert(hp->seq == INT_MIN || hp->seq == INT_MAX || hp->seq >= -5 || hp->seq <= 5, "invalid handler, seq = %d", hp->seq);
      if (hp->event == _EVENT_NULL || hp->event == ev.event) {
        _Context *next = hp->handler(ev, context);
        if (next) ret = next;
      }
    }
    CLog(FG_PURPLE, "<<<<<< OUT OF TRAP");
    spinlock_release(&os_trap_lock);

    Assert(ret, "returning to a null context after trap");
    return ret;
  }
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
