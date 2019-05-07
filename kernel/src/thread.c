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

struct task root_task;
struct alarm_log alarm_head;
struct spinlock *wakeup_reacquire_lock = NULL;

_Context *null_contexts[MAX_CPU] = {};
struct task *cpu_tasks[MAX_CPU] = {};
struct task *get_current_task() {
  Assert(cpu_tasks[_cpu()] != &root_task, "cannot tun as root-task");
  return cpu_tasks[_cpu()];
}
void set_current_task(struct task *task) {
  Assert(task != &root_task, "cannot set as root-task");
  cpu_tasks[_cpu()] = task;
}

void kmt_init() {
  wakeup_reacquire_lock = NULL;

  root_task.pid   = next_pid++;
  root_task.name  = "Root Task";
  root_task.state = ST_X;
  root_task.next  = NULL;
  memset(root_task.fenceA, FILL_FENCE, sizeof(root_task.fenceA));
  memset(root_task.stack,  FILL_STACK, sizeof(root_task.stack));
  memset(root_task.fenceB, FILL_FENCE, sizeof(root_task.fenceB));
  kmt_inspect_fence(&root_task);

  // add trap handlers
  os->on_irq(INT_MIN, _EVENT_NULL,      kmt_context_save);
  os->on_irq(-1,      _EVENT_ERROR,     kmt_error);
  os->on_irq(0,       _EVENT_YIELD,     kmt_yield);
  os->on_irq(0,       _EVENT_IRQ_TIMER, kmt_yield);
  os->on_irq(1,       _EVENT_SYSCALL,   do_syscall);
  os->on_irq(INT_MAX, _EVENT_NULL,      kmt_context_switch);
}

int kmt_create(struct task *task, const char *name, void (*entry)(void *arg), void *arg) {
  task->pid   = next_pid++;
  task->name  = name;
  task->entry = entry;
  task->arg   = arg;
  task->state = ST_E;
  task->owner = -1;
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
  task->context = _kcontext(stack, entry, arg);

  struct task *tp = &root_task;
  while (tp->next) tp = tp->next;
  tp->next = task;

  return task->pid;
}

void kmt_teardown(struct task *task) {
  struct task *tp = &root_task;
  while (tp->next && tp->next != task) tp = tp->next;
  Assert(tp->next && tp->next == task, "Task is not in linked list!");
  tp->next = task->next;
  pmm->free(task);
}

void kmt_inspect_fence(struct task *task) {
  Assert(memcmp(const_fence, task->fenceA, sizeof(const_fence)) == 0, "Fence inspection A for task %d (%s) failed.", task->pid, task->name);
  Assert(memcmp(const_fence, task->fenceB, sizeof(const_fence)) == 0, "Fence inspection B for task %d (%s) failed.", task->pid, task->name);
}

_Context *kmt_context_save(_Event ev, _Context *context) {
  struct task *cur = get_current_task();
  if (cur) {
    Assert(!cur->context, "double context saving for task %d: %s", cur->pid, cur->name);
    cur->state   = ST_W;
    cur->owner   = -1;
    cur->context = context;
  } else {
    Assert(!null_contexts[_cpu()], "double context saving for null context");
    null_contexts[_cpu()] = context;
  }
  return NULL;
}

_Context *kmt_context_switch(_Event ev, _Context *context) {
  _Context *ret = NULL;
  struct task *cur = get_current_task();
  if (cur) {
    Log("Next is %d: %s", cur->pid, cur->name);
    kmt_inspect_fence(cur);
    Assert(cur->owner == -1, "switching to an already running task");
    ret = cur->context;
    cur->state   = ST_R;
    cur->owner   = _cpu();
    cur->context = NULL;
    if (cur->alarm) {
      Assert(cur->lock, "No lock to reacquire");
      CLog(FG_YELLOW, "will reacquire lock %s", cur->lock->name);
      wakeup_reacquire_lock = cur->lock;
      cur->alarm = NULL;
      cur->lock  = NULL;
    }
    cur->count   = cur->count >= 1000 ? 0 : cur->count + 1;
    Assert(ret, "task context is empty");
  } else {
    Log("Next is NULL task");
    ret = null_contexts[_cpu()];
    null_contexts[_cpu()] = NULL;
    Assert(ret, "null context is empty");
  }
  return ret;
}

struct task *kmt_sched() {
  Log("========== TASKS ==========");
  struct task *ret = NULL;
  for (struct task *tp = &root_task; tp != NULL; tp = tp->next) {
    kmt_inspect_fence(tp);
    Log("%d:%s [%s, L%d, C%d]", tp->pid, tp->name, task_states_human[tp->state], tp->owner, tp->count);
    if (tp->state == ST_E || tp->state == ST_W) {
      if (!ret || tp->count < ret->count) {
        ret = tp;
      }
    }
  }
  Log("===========================");
  if (ret == get_current_task()) ret = NULL;
  return ret;
}

_Context *kmt_yield(_Event ev, _Context *context) {
  set_current_task(kmt_sched());
  return NULL;
}

_Context *kmt_error(_Event ev, _Context *context) {
  Assert(ev.event = _EVENT_ERROR, "Not an error interrupt");
  printf("\nError detected on CPU %d:\n>>> %s <<<\n", _cpu(), ev.msg);
  printf("====================\n");
  printf("EIP was %p\n", context->eip);
  printf("ESP was %p\n", context->esp0);
  printf("RET ADDR %p\n", *((void **) context->esp0));
  printf("====================\n");
  printf("Current tasks:\n");
  for (int i = 0; i < _ncpu(); ++i) {
    printf("[CPU%d] ", i);
    if (cpu_tasks[i]) {
      printf("%d: %s", cpu_tasks[i]->pid, cpu_tasks[i]->name);
    } else {
      printf("NULL");
    }
    printf("\n");
  }
  Panic("Error detected: %s", ev.msg);
  return NULL;
}

uintptr_t kmt_sleep(void *alarm, struct spinlock *lock) {
  struct task *cur = get_current_task();
  Assert(cur,   "NULL task is going to sleep.");
  Assert(alarm, "Sleep without a alarm (semaphore).");

  // even if the task does not sleep,
  // the lock must be saved so that it
  // will be reacquired when trap returns
  cur->alarm = alarm;
  cur->lock  = lock;

  bool already_alarmed = false;
  struct alarm_log *ap = alarm_head.next;
  struct alarm_log *an = NULL;
  alarm_head.next = NULL;
  while (ap) {
    an = ap->next;
    if (ap->alarm == alarm) {
      already_alarmed = true;
      pmm->free(ap);
    } else {
      ap->next = alarm_head.next;
      alarm_head.next = ap;
    }
    ap = an;
  }

  if (true || already_alarmed) {
    CLog(FG_YELLOW, "No sleep");
    return -1;
  } else {
    cur->state = ST_S;
    set_current_task(kmt_sched());
    return 0;
  }
}

uintptr_t kmt_wakeup(void *alarm) {
  struct task* cur = get_current_task();

  // avoid reinsertion
  bool already_alarmed = false;
  for (struct alarm_log *ap = alarm_head.next; ap != NULL; ap = ap->next) {
    if (ap->alarm == alarm) {
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

  for (struct task *tp = root_task.next; tp != NULL; tp = tp->next) {
    if (tp->state == ST_S && tp->alarm == alarm) {
      CLog(FG_YELLOW, "waked up task %d: %s", tp->pid, tp->name);
      tp->state = ST_W; // wake up
    }
  }
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
