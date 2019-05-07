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
  volatile int *pval = &sem->value;
  while (*pval <= 0) {
    uintptr_t res = 0;
    asm volatile ("int $0x80" 
        : "=a"(res) 
        : "0"(SYS_sleep), "b"(sem), "c"(&sem->lock)
        );
  }
  Assert(spinlock_holding(&sem->lock), "Not holding the lock after waking up");
  __sync_synchronize();
  --sem->value;
  spinlock_release(&sem->lock);
}

void semaphore_signal(struct semaphore *sem) {
  spinlock_acquire(&sem->lock);
  ++sem->value;
  __sync_synchronize();
  uintptr_t res = 0;
  asm volatile ("int $0x80" 
      : "=a"(res) 
      : "0"(SYS_wakeup), "b"(sem)
      );
  spinlock_release(&sem->lock);
}
