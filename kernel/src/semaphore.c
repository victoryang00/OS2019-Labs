#include <common.h>
#include <os.h>
#include <thread.h>
#include <spinlock.h>
#include <semaphore.h>
#include <debug.h>

extern struct spinlock os_trap_lock;
extern struct task **cpu_tasks;
extern struct task root_task;

void semaphore_init(struct semaphore *sem, const char *name, int value) {
  spinlock_init(&sem->lock, name);
  sem->name = name;
  sem->value = value;
}

void semaphore_wait(struct semaphore *sem) {
  Assert(!spinlock_holding(&os_trap_lock), "no semaphore wait in trap");
  spinlock_acquire(&sem->lock);
  while (sem->value <= 0) {
    Assert(!spinlock_holding(&os_trap_lock), "sleep in trap");

    // release the lock first to prevent deadlock
    spinlock_release(&sem->lock);
    spinlock_acquire(&os_trap_lock);
    struct task *cur = get_current_task();
    Assert(cur, "in semaphore, no task");
    cur->alarm = sem;
    spinlock_release(&os_trap_lock);

    _yield();
    spinlock_acquire(&sem->lock);
  }
  __sync_synchronize();
  --sem->value;
  spinlock_release(&sem->lock);
}

void semaphore_signal(struct semaphore *sem) {
  spinlock_acquire(&sem->lock);
  ++sem->value;
  spinlock_release(&sem->lock);

  bool holding = spinlock_holding(&os_trap_lock);
  if (!holding) spinlock_acquire(&os_trap_lock);
  for (struct task *tp = root_task.next; tp != NULL; tp = tp->next) {
    if (tp->alarm == sem) {
      if (tp->state == ST_S) tp->state = ST_W;
      tp->alarm = NULL; // stop going to sleep
    }
  }
  if (!holding) spinlock_release(&os_trap_lock);
}
