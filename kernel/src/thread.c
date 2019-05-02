#include <common.h>
#include <thread.h>
#include <spinlock.h>
#include <semaphore.h>

/**
 * Kernel Multi-Thread module (KMT, Proc)
 */

static int next_pid = 1;
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

struct spinlock task_lock;
struct task root_task;

static struct task *cpu_tasks[MAX_CPU] = {};
static inline struct task *get_current_task() {
  return cpu_tasks[_cpu()];
}
static inline void set_current_task(struct task *task) {
  cpu_tasks[_cpu()] = task;
}

void kmt_init() {
  spinlock_init(&task_lock, "KMT(Proc) Lock");
  
  __sync_synchronize();

  spinlock_acquire(&task_lock);
  root_task.pid = next_pid++;
  root_task.name = "Root Task";
  root_task.state = ST_X;
  root_task.next = NULL;
  spinlock_release(&task_lock);

  // add trap handlers
  Log("%d", -1);
  os->on_irq(INT_MIN, _EVENT_NULL, kmt_context_save);
  os->on_irq(INT_MAX, _EVENT_NULL, kmt_context_switch);
  //os->on_irq(0, _EVENT_YIELD, kmt_yield);
}

int kmt_create(struct task *task, const char *name, void (*entry)(void *arg), void *arg) {
  task->pid = next_pid++;
  task->name = name;
  task->context = kcontext(task->stack, entry, arg);
  memset(task->fenceA, FILL_FENCE, sizeof(task->fenceA));
  memset(task->stack,  FILL_STACK, sizeof(task->stack));
  memset(task->fenceB, FILL_FENCE, sizeof(task->fenceB));

  task->next = NULL;

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

static _Context *saved_context = NULL;
_Context *kmt_context_save(_Event ev, _Context *context) {
  Log("KMT Context Save");
  saved_context = context;
  return NULL;
}

_Context *kmt_context_switch(_Event ev, _Context *context) {
  Log("KMT Context Switch");
  _Context *ret = saved_context;
  saved_context = NULL;
  return ret;
}

void kmt_sched() {
  Assert(spinlock_holding(&task_lock), "The task is not holding task lock.");
  Assert(get_current_task()->state != ST_R, "The task is still running.");
  Assert((get_efl() & FL_IF) == 0, "The CPU is interruptable.");

  for (struct task *tp = &root_task; tp != NULL; tp = tp->next) {
    if (tp->state == ST_W) { // choose a waken up task
      
    }
  }
}

void kmt_yield() {
  spinlock_acquire(&task_lock);
  get_current_task()->state = ST_W; // give up CPU
  kmt_sched(); // call scheduler to switch to next task
  spinlock_release(&task_lock);
}

void kmt_sleep(void *alarm, struct spinlock *lock) {
  struct task *cur = get_current_task();
  Assert(cur != NULL, "NULL task is going to sleep.");
  Assert(alarm != NULL, "Sleep without a alarm (semaphore).");
  Assert(lock != NULL, "Sleep without a lock.");

  // MUST acquire the tasks lock
  // before giving up the one we are holding
  // OTHERWISE MAY MISS WAKEUPS (DEADLOCK)
  // TODO: IS THERE OTHER TYPES OF DEADLOCK?
  if (lock != &task_lock) {
    spinlock_acquire(&task_lock);
    spinlock_release(lock);
  }

  cur->alarm = alarm;
  cur->state = ST_S; // go to sleep
  kmt_sched(); // lock will be released

  __sync_synchronize(); // memory barrier
  cur->alarm = NULL; // turn off the alarm
  
  // We have the task lock when wake up
  // then we need to acquire the original lock
  if (lock != &task_lock) {
    spinlock_release(&task_lock);
    spinlock_acquire(lock);
  }
}

void kmt_wakeup(void *alarm) {
  spinlock_acquire(&task_lock);
  for (struct task *tp = &root_task; tp != NULL; tp = tp->next) {
    if (tp->state == ST_S && tp->alarm == alarm) {
      tp->state = ST_W; // wake up
    }
  }
  spinlock_release(&task_lock);
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
