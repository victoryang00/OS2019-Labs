#include <common.h>
#include <spinlock.h>
#include <semaphore.h>
#include <debug.h>

void semaphore_init(struct semaphore *sem, const char *name, int value) {
  spinlock_init(&sem->lock, "Semaphore Lock");

  // initialize the lock before changing other properties
  __sync_synchronize();

  spinlock_acquire(&sem->lock);
  sem->name = name;
  sem->value = value;
  spinlock_release(&sem->lock);
}

void semaphore_wait(struct semaphore *sem) {
  spinlock_acquire(&sem->lock);
  while (sem->value < 0) {
    // TODO:
    // sleep(sem, &sem->lock);
  }
  --sem->value;
  spinlock_release(&sem->lock);
}

void semaphore_signal(struct semaphore *sem) {
  spinlock_acquire(&sem->lock);
  ++sem->value;
  if (sem->value > 0) {
    // TODO:
    // wakeup(sem);
  }
  spinlock_release(&sem->lock);
}
