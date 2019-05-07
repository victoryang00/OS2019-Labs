#include <common.h>
#include <os.h>
#include <thread.h>
#include <spinlock.h>
#include <semaphore.h>
#include <debug.h>

void semaphore_init(struct semaphore *sem, const char *name, int value) {
  spinlock_init(&sem->lock, name);
  sem->name = name;
  sem->value = value;
}

void semaphore_wait(struct semaphore *sem) {
  spinlock_acquire(&sem->lock);
  Assert(!spinlock_holding(&os_trap_lock), "sleep in trap");
  while (sem->value <= 0) {
    spinlock_acquire(&os_trap_lock);
    struct task *cur = get_current_task();
    cur->state = ST_T;
    cur->alarm = sem;
    spinlock_release(&os_trap_lock);

    spinlock_release(&sem->lock);
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
  __sync_synchronize();
  bool holding = spinlock_holding(&os_trap_lock);
  if (!holding) spinlock_acquire(&os_trap_lock);
  for (struct task *tp = root_task.next; tp != NULL; tp = tp->next) {
    if (tp->state == ST_S && tp->alarm == sem) {
      tp->state = ST_W;
    }
  }
  if (!holding) spinlock_release(&os_trap_lock);
  spinlock_release(&sem->lock);
}
