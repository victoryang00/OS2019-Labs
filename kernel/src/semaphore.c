#include <common.h>
#include <thread.h>
#include <syscall.h>
#include <spinlock.h>
#include <semaphore.h>
#include <debug.h>

extern struct spinlock os_trap_lock;

void semaphore_init(struct semaphore *sem, const char *name, int value) {
  spinlock_init(&sem->lock, "Sem Lock");
  sem->name = name;
  sem->value = value;
}

void semaphore_wait(struct semaphore *sem) {
  spinlock_acquire(&sem->lock);
  printf("-[%s = %d]\n", sem->name, sem->value);
  while (sem->value <= 0) {
    // hold the lock so that it will not be interrupted
    spinlock_acquire(&os_trap_lock);
    spinlock_release(&sem->lock);
    asm volatile ("int $0x80" : : "a"(SYS_sem_wait), "b"(sem), "c"(&sem->lock));
    spinlock_acquire(&sem->lock);
    spinlock_release(&os_trap_lock);
    printf("-[%s = %d]\n", sem->name, sem->value);
  }
  __sync_synchronize();
  --sem->value;
  spinlock_release(&sem->lock);
}

void semaphore_signal(struct semaphore *sem) {
  spinlock_acquire(&sem->lock);
  ++sem->value;
  __sync_synchronize();
  printf("+[%s = %d]\n", sem->name, sem->value);
  asm volatile ("int $0x80" : : "a"(SYS_sem_signal), "b"(sem));
  spinlock_release(&sem->lock);
}
