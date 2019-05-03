#include <common.h>
#include <thread.h>
#include <spinlock.h>
#include <semaphore.h>
#include <debug.h>

void semaphore_init(struct semaphore *sem, const char *name, int value) {
  spinlock_init(&sem->lock, "Semaphore Lock");
  sem->name = name;
  sem->value = value;
}

void semaphore_wait(struct semaphore *sem) {
  spinlock_acquire(&sem->lock);
  Log("Waiting for semaphore %s (%d)", sem->name, sem->value);
  while (sem->value <= 0) {
    kmt_sleep((void *) sem, &sem->lock);
  }
  --sem->value;
  spinlock_release(&sem->lock);
}

void semaphore_signal(struct semaphore *sem) {
  spinlock_acquire(&sem->lock);
  ++sem->value;
  kmt_wakeup((void *) sem);
  Log("Signaled for semaphore %s (%d)", sem->name, sem->value);
  spinlock_release(&sem->lock);
}
