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
  Log("%d", -2147483648);
  os->on_irq(INT_MIN, _EVENT_NULL, kmt_context_save);
  os->on_irq(INT_MAX, _EVENT_NULL, kmt_context_switch);
}

int kmt_create(struct task *task, const char *name, void (*entry)(void *arg), void *arg) {
  task->pid = next_pid++;
  task->name = name;
  task->context.eax = (uint32_t) arg;
  task->context.eip = (uint32_t) entry;
  task->context.ebp = (uint32_t) task->stack;
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

_Context *kmt_context_save(_Event ev, _Context *context) {
  Log("KMT Context Save");
  return NULL;
}

_Context *kmt_context_switch(_Event ev, _Context *context) {
  Log("KMT Context Switch");
  return NULL;  
}

void kmt_sched() {
  //?????
}

void kmt_yield() {
  spinlock_acquire(&task_lock);
  get_current_task()->state = ST_W; // give up CPU
  kmt_sched(); // call scheduler to switch to next task
  spinlock_release(&task_lock);
}

void kmt_sleep(void *sem, struct spinlock *lock) {
  struct task *cur = get_current_task();
  Assert(cur != NULL, "NULL task is going to sleep.");
  Assert(sem != NULL, "Sleep without a semaphore.");
  Assert(lock != NULL, "Sleep without a lock.");


}

void kmt_wakeup(void *sem) {
  spinlock_acquire(&task_lock);
  for (struct task *tp = &root_task; tp != NULL; tp = tp->next) {
    if (tp->state == ST_S && tp->sleep_sem == sem) {
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
