#include <common.h>
#include <thread.h>
#include <syscall.h>
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
  while (sem->value <= 0) {
    asm volatile ("int $0x80" : : "a"(SYS_sleep), "b"(sem), "c"(&sem->lock));
  }
  Assert(spinlock_holding(&sem->lock), "Not holding the lock after waking up");
  __sync_synchronize();
  --sem->value;
  spinlock_release(&sem->lock);
}

void semaphore_signal(struct semaphore *sem) {
  printf("before signal on CPU %d\n", _cpu());
  spinlock_acquire(&sem->lock);
  printf("signal lock acquired on CPU %d\n", _cpu());
  ++sem->value;
  __sync_synchronize();
  asm volatile ("int $0x80" : : "a"(SYS_wakeup), "b"(sem));
  printf("signal OK of CPU %d\n", _cpu());
  spinlock_release(&sem->lock);
  printf("signal lock released on CPU %d\n", _cpu());
}
