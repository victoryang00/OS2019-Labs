#include <common.h>
#include <thread.h>
#include <syscall.h>
#include <spinlock.h>
#include <semaphore.h>

/**
 * Kernel Multi-Thread module (KMT, Proc)
 */

static uint32_t next_pid = 1;
static const char const_fence[32] = { 
  FILL_FENCE, FILL_FENCE, FILL_FENCE, FILL_FENCE,
  FILL_FENCE, FILL_FENCE, FILL_FENCE, FILL_FENCE,
  FILL_FENCE, FILL_FENCE, FILL_FENCE, FILL_FENCE,
  FILL_FENCE, FILL_FENCE, FILL_FENCE, FILL_FENCE,
  FILL_FENCE, FILL_FENCE, FILL_FENCE, FILL_FENCE,
  FILL_FENCE, FILL_FENCE, FILL_FENCE, FILL_FENCE,
  FILL_FENCE, FILL_FENCE, FILL_FENCE, FILL_FENCE,
  FILL_FENCE, FILL_FENCE, FILL_FENCE, FILL_FENCE
};
static const char *task_states_human[8] __attribute__((used)) = {
  "Unused",
  "Embryo",
  "To sleep",
  "Sleeping",
  "Waken up",
  "Running",
  "Zombie",
  "Special"
};

struct spinlock task_lock;
struct task root_task;
struct alarm_log alarm_head;

struct task *cpu_tasks[MAX_CPU] = {};
static inline struct task *get_current_task() {
  return cpu_tasks[_cpu()];
}
static inline void set_current_task(struct task *task) {
  task->owner = _cpu();
  cpu_tasks[_cpu()] = task;
}

void kmt_init() {
  memset(cpu_tasks, 0x00, sizeof(cpu_tasks));
  spinlock_init(&task_lock,       "Task Lock");
  
  __sync_synchronize();

  spinlock_acquire(&task_lock);
  root_task.pid   = next_pid++;
  root_task.name  = "Root Task";
  root_task.state = ST_X;
  root_task.next  = NULL;
  memset(root_task.fenceA, FILL_FENCE, sizeof(root_task.fenceA));
  memset(root_task.stack,  FILL_STACK, sizeof(root_task.stack));
  memset(root_task.fenceB, FILL_FENCE, sizeof(root_task.fenceB));
  kmt_inspect_fence(&root_task);
  spinlock_release(&task_lock);

  // add trap handlers
  os->on_irq(INT_MIN, _EVENT_NULL,      kmt_context_save);
  os->on_irq(-1,      _EVENT_ERROR,     kmt_error);
  os->on_irq(0,       _EVENT_YIELD,     kmt_yield);
  os->on_irq(1,       _EVENT_IRQ_TIMER, kmt_yield);
  os->on_irq(2,       _EVENT_SYSCALL,   do_syscall);
  os->on_irq(INT_MAX, _EVENT_NULL,      kmt_context_switch);
}

int kmt_create(struct task *task, const char *name, void (*entry)(void *arg), void *arg) {
  task->pid   = next_pid++;
  task->name  = name;
  task->entry = entry;
  task->arg   = arg;
  task->state = ST_E;
  task->count = 0;
  memset(task->fenceA, FILL_FENCE, sizeof(task->fenceA));
  memset(task->stack,  FILL_STACK, sizeof(task->stack));
  memset(task->fenceB, FILL_FENCE, sizeof(task->fenceB));
  kmt_inspect_fence(task);
  task->alarm = NULL;
  task->next  = NULL;

  /**
   * We cannot create context before initializing the stack
   * because kcontext will put the context at the begin of stack
   */
  _Area stack = { 
    (void *) task->stack, 
    (void *) task->stack + sizeof(task->stack) 
  };
  struct context_item *cp = pmm->alloc(sizeof(struct context_item));
  cp->context = _kcontext(stack, entry, arg);
  cp->next = NULL;
  task->context_head.next = cp;

  spinlock_acquire(&task_lock);
  struct task *tp = &root_task;
  while (tp->next) tp = tp->next;
  tp->next = task;
  spinlock_release(&task_lock);

  return task->pid;
}

void kmt_teardown(struct task *task) {
  spinlock_acquire(&task_lock);
  struct task *tp = &root_task;
  while (tp->next && tp->next != task) tp = tp->next;
  Assert(tp->next && tp->next == task, "Task is not in linked list!");
  tp->next = task->next;
  spinlock_release(&task_lock);

  pmm->free(task);
}

void kmt_inspect_fence(struct task *task) {
  Assert(memcmp(const_fence, task->fenceA, sizeof(const_fence)) == 0, "Fence inspection A for task %d (%s) failed.", task->pid, task->name);
  Assert(memcmp(const_fence, task->fenceB, sizeof(const_fence)) == 0, "Fence inspection B for task %d (%s) failed.", task->pid, task->name);
}

_Context *kmt_context_save(_Event ev, _Context *context) {
  //Log("KMT Context Save");
  spinlock_acquire(&task_lock);
  struct task *cur = get_current_task();
  if (cur) {
    struct context_item *cp = pmm->alloc(sizeof(struct context_item));
    cp->context = context;
    cp->next = cur->context_head.next;
    cur->context_head.next = cp;
    Log("Context for task %d: %s saved.", cur->pid, cur->name);
  }
  spinlock_release(&task_lock);
  return NULL;
}

_Context *kmt_context_switch(_Event ev, _Context *context) {
  //Log("KMT Context Switch");
  spinlock_acquire(&task_lock);
  _Context *ret = NULL;
  struct task *cur = get_current_task();
  if (cur) {
    kmt_inspect_fence(cur);
    struct context_item *cp = cur->context_head.next;
    Assert(cp, "task has no context to load");
    cur->context_head.next = cp->next;
    ret = cp->context;
    pmm->free(cp);
    Log("Context for task %d: %s loaded.", cur->pid, cur->name);
  } else {
    ret = context;
  }
  Assert(ret, "BAD SAVED CONTEXT");
  spinlock_release(&task_lock);
  return ret;
}

struct task *kmt_sched() {
  Assert(spinlock_holding(&task_lock), "Not holding the task lock!");
  Log("========== TASKS ==========");
  struct task *ret = NULL;
  //printf("\ninterrupt by cpu %d\n", _cpu());
  //struct task *cur = NULL;
  //if ((cur = get_current_task()) != NULL) {
  //  printf("interrupter task is %d:%s", cur->pid, cur->name);
  //}
  for (struct task *tp = &root_task; tp != NULL; tp = tp->next) {
    kmt_inspect_fence(tp);
    Log("%d:%s [%s, L%d, C%d]", tp->pid, tp->name, task_states_human[tp->state], tp->owner, tp->count);
    if (tp->state == ST_E || tp->state == ST_W) {  // choose a waken up task
      if (ret == NULL || tp->count < ret->count) { // a least ran one
        ret = tp;
      }
    }
  }
  Log("===========================");
  return ret;
}

_Context *kmt_yield(_Event ev, _Context *context) {
  spinlock_acquire(&task_lock);
  struct task *cur = get_current_task();
  if (cur->state == ST_T) {
    // going to sleep. this shall override.
    spinlock_release(&task_lock);
    return NULL; // no change!
  }

  struct task *next = kmt_sched(); // call scheduler
  if (!next) {
    Log("No scheduling is made.");
    if (cur) {
      cur->count = cur->count >= 1000 ? 0 : cur->count + 1;
    }
  } else {
    Log("Switching to task %d:%s", next->pid, next->name);
    //Log("Entry: %p", next->context->eip);
    if (cur) {
      if (cur->state == ST_R) {
        cur->state = ST_W;
      }
    }
    Assert(next->state != ST_T, "switching to a task that is going to sleep.");
    next->state = ST_R; // set the next as running
    next->count = next->count >= 1000 ? 0 : next->count + 1;
    set_current_task(next);
  }
  spinlock_release(&task_lock);
  return NULL;
}

_Context *kmt_error(_Event ev, _Context *context) {
  Assert(ev.event = _EVENT_ERROR, "Not an error interrupt");
  printf("\nError detected: %s\n", ev.msg);
  //CLog(BG_RED, "Error detected: %s", ev.msg);
  return NULL;
}

uintptr_t kmt_sem_sleep(void *alarm) {
  struct task *cur = get_current_task();
  Assert(cur != NULL, "NULL task is going to sleep.");
  Assert(cur->state == ST_T || cur->state == ST_R, "Task in wrong state %s instead of ST_T.", task_states_human[cur->state]);
  Assert(alarm != NULL, "Sleep without a alarm (semaphore).");

  spinlock_acquire(&task_lock);
  bool already_alarmed = false;
  struct alarm_log *ap = alarm_head.next;
  struct alarm_log *an = NULL;
  alarm_head.next = NULL;
  while (ap) {
    an = ap->next;
    if (ap->alarm == alarm) already_alarmed = true;
    if (ap->alarm != alarm && ap->issuer != cur) {
      pmm->free(ap);
    } else {
      ap->next = alarm_head.next;
      alarm_head.next = ap;
    }
    ap = an;
  }

  struct task *next = kmt_sched();
  if (!next || already_alarmed) {
    CLog(FG_YELLOW, "No next task or already alarmed");
    cur->state = ST_R;
    cur->count = cur->count >= 1000 ? 0 : cur->count + 1; 
  } else {
    CLog(FG_YELLOW, "Next is %d: %s", next->pid, next->name);
    cur->state = ST_S;
    cur->alarm = alarm;
    Assert(next->state != ST_T, "switching to a task that is going to sleep.");
    next->state = ST_R;
    next->count = next->count >= 1000 ? 0 : next->count + 1;
    set_current_task(next);
  }
  spinlock_release(&task_lock);
  return 0;
}

uintptr_t kmt_sem_wakeup(void *alarm) {
  spinlock_acquire(&task_lock);
  struct task* cur = get_current_task();

  // avoid reinsertion
  bool already_alarmed = false;
  for (struct alarm_log *ap = alarm_head.next; ap != NULL; ap = ap->next) {
    if (ap->alarm == alarm && ap->issuer == cur) {
      already_alarmed = true;
      break;
    }
  }
  if (!already_alarmed) {
    struct alarm_log *ap = pmm->alloc(sizeof(struct alarm_log));
    ap->alarm  = alarm;
    ap->issuer = cur;
    ap->next   = alarm_head.next;
    alarm_head.next = ap;
  }

  for (struct task *tp = &root_task; tp != NULL; tp = tp->next) {
    if (tp->state == ST_S && tp->alarm == alarm) {
      CLog(FG_YELLOW, "waked up task pid %d", tp->pid);
      tp->state = ST_W; // wake up
    }
  }
  spinlock_release(&task_lock);
  return 0;
}

MODULE_DEF(kmt) {
  .init        = kmt_init,
  .create      = kmt_create,
  .teardown    = kmt_teardown,
  .spin_init   = spinlock_init,
  .spin_lock   = spinlock_acquire,
  .spin_unlock = spinlock_release,
  .sem_init    = semaphore_init,
  .sem_wait    = semaphore_wait,
  .sem_signal  = semaphore_signal
};
