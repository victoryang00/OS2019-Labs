#include <common.h>
#include <thread.h>
#include <spinlock.h>
#include <semaphore.h>

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

extern struct spinlock os_trap_lock; 

struct task root_task;
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
  root_task.pid   = next_pid++;
  root_task.name  = "Root Task";
  root_task.state = ST_X;
  root_task.next  = NULL;
  root_task.count = 0;
  memset(root_task.fenceA, FILL_FENCE, sizeof(root_task.fenceA));
  memset(root_task.stack,  FILL_STACK, sizeof(root_task.stack));
  memset(root_task.fenceB, FILL_FENCE, sizeof(root_task.fenceB));
  kmt_inspect_fence(&root_task);

  // add trap handlers
  os->on_irq(INT_MIN, _EVENT_NULL,      kmt_context_save);
  os->on_irq(0,       _EVENT_ERROR,     kmt_error);
  os->on_irq(0,       _EVENT_IRQ_TIMER, kmt_timer);
  os->on_irq(0,       _EVENT_YIELD,     kmt_yield);
  os->on_irq(INT_MAX, _EVENT_NULL,      kmt_context_switch);
}

int kmt_create(struct task *task, const char *name, void (*entry)(void *arg), void *arg) {
  task->pid     = next_pid++;
  task->name    = name;
  task->entry   = entry;
  task->arg     = arg;
  task->state   = ST_E;
  task->owner   = -1;
  task->count   = 0;
  task->alarm   = NULL;
  task->suicide = 0;
  task->next    = NULL;

  // We cannot create context before initializing the stack
  // since context will put the context at the begin of stack
  memset(task->fenceA, FILL_FENCE, sizeof(task->fenceA));
  memset(task->stack,  FILL_STACK, sizeof(task->stack));
  memset(task->fenceB, FILL_FENCE, sizeof(task->fenceB));
  kmt_inspect_fence(task);
  _Area stack = { 
    (void *) task->stack, 
    (void *) task->stack + sizeof(task->stack) 
  };
  task->context = _kcontext(stack, entry, arg);

  bool holding = spinlock_holding(&os_trap_lock);
  if (!holding) spinlock_acquire(&os_trap_lock);
  struct task *tp = &root_task;
  while (tp->next) tp = tp->next;
  tp->next = task;
  if (!holding) spinlock_release(&os_trap_lock);

  return task->pid;
}

void kmt_teardown(struct task *task) {
  bool holding = spinlock_holding(&os_trap_lock);
  if (!holding) spinlock_acquire(&os_trap_lock);
  task->suicide = 1;
  if (!holding) spinlock_release(&os_trap_lock);
}

void kmt_inspect_fence(struct task *task) {
  Assert(memcmp(const_fence, task->fenceA, sizeof(const_fence)) == 0, "Fence inspection A for task %d (%s) failed.", task->pid, task->name);
  Assert(memcmp(const_fence, task->fenceB, sizeof(const_fence)) == 0, "Fence inspection B for task %d (%s) failed.", task->pid, task->name);
}

struct task *kmt_sched() {
  // zombie task conducts suicide
  struct task *cur = get_current_task();
  if (cur->suicide) {
    struct task *tp = &root_task;
    while (tp && tp->next != cur) tp = tp->next;
    Assert(tp, "task not in task list");
    tp->next = cur->next;
    pmm->free(cur);
  }

  // pick a next task
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
  return ret;
}

_Context *kmt_context_save(_Event ev, _Context *context) {
  Assert(spinlock_holding(&os_trap_lock), "not holding os trap lock");
  struct task *cur = get_current_task();
  if (cur) {
    Assert(!cur->context, 
        "double context saving for task %d: %s [%s]", 
        cur->pid, cur->name, task_states_human[cur->state]);
    if (context->esp0 < (uintptr_t) cur->stack
        || context->esp0 > ((uintptr_t) cur->stack) + sizeof(cur->stack)) {
      printf("ESP not in stack area when saving it.\n");
      printf("Context is located at %p\n", context);
      printf("ESP is %p\n", context->esp0);
      printf("stack for %d: %s is [%p, %p]\n", cur->pid, cur->name, cur->stack, cur->stack + sizeof(cur->stack));
      Panic("ESP not in stack area when saving it.");
    }

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
  Assert(spinlock_holding(&os_trap_lock), "not holding os trap lock");
  _Context *ret = NULL;
  struct task *cur = get_current_task();
  if (cur) {
    Log("Next is %d: %s", cur->pid, cur->name);
    kmt_inspect_fence(cur);
    Assert(cur->owner == -1, "switching to an already running task");
    cur->state   = ST_R;
    cur->owner   = _cpu();
    ret = cur->context;
    cur->context = NULL;
    Assert(ret, "task context is empty");
    cur->count   = cur->count >= 1000 ? 0 : cur->count + 1;
    if (ret->esp0 < (uintptr_t) cur->stack
        || ret->esp0 > ((uintptr_t) cur->stack) + sizeof(cur->stack)) {
      printf("ESP not in stack area when loading it.\n");
      printf("ESP is %p\n", ret->esp0);
      printf("stack for %d: %s is [%p, %p]\n", cur->pid, cur->name, cur->stack, cur->stack + sizeof(cur->stack));
      Panic("ESP not in stack area when loading it.");
    }
  } else {
    Log("Next is NULL task");
    ret = null_contexts[_cpu()];
    null_contexts[_cpu()] = NULL;
    Assert(ret, "null context is empty");
  }
  return ret;
}

_Context *kmt_timer(_Event ev, _Context *context) {
  // stupid, simple deadlock preventor
  ++root_task.count;
  if (root_task.count >= 1500) {
    root_task.count = 0;
    for (struct task *tp = root_task.next; tp != NULL; tp = tp->next) {
      if (tp->state == ST_S) {
        tp->state = ST_W;
        tp->alarm = NULL;
      }
    }
  }
  set_current_task(kmt_sched());
  return NULL;
}

_Context *kmt_yield(_Event ev, _Context *context) {
  Assert(spinlock_holding(&os_trap_lock), "not holding os trap lock");
  struct task *cur = get_current_task();
  if (cur && cur->alarm) {
    cur->state = ST_S;  
  }
  set_current_task(kmt_sched());
  return NULL;
}

_Context *kmt_error(_Event ev, _Context *context) {
  Assert(spinlock_holding(&os_trap_lock), "not holding os trap lock");
  Assert(ev.event = _EVENT_ERROR, "Not an error interrupt");
  printf("\nError detected on CPU %d:\n>>> %s <<<\n", _cpu(), ev.msg);
  printf("====================\n");
  printf("Context at %p\n", context);
  printf("EIP was %p\n", context->eip);
  printf("ESP was %p\n", context->esp0);
  printf("RET ADDR %p\n", *((void **) context->esp0));
  printf("====================\n");
  printf("Current tasks:\n");
  for (int i = 0; i < _ncpu(); ++i) {
    printf("[CPU%d] ", i);
    if (cpu_tasks[i]) {
      printf("%d: %s ", cpu_tasks[i]->pid, cpu_tasks[i]->name);
      printf("stack [%p, %p]", cpu_tasks[i]->stack, cpu_tasks[i]->stack + sizeof(cpu_tasks[i]->stack));
    } else {
      printf("(no task)");
    }
    printf("\n");
  }
  
  // kernel threads cannot be killed
  Panic("Fatal error detected: %s.", ev.msg);
  return NULL;
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
