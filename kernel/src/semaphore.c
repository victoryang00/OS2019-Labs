#include <common.h>
#include <thread.h>
#include <syscall.h>
#include <spinlock.h>
#include <semaphore.h>
#include <debug.h>

void semaphore_init(struct semaphore *sem, const char *name, int value) {
  sprintf(sem->fence, "FUCK");
  spinlock_init(&sem->lock, "Sem Lock");
  sem->name = name;
  sem->value = value;
}

void semaphore_wait(struct semaphore *sem) {
  spinlock_acquire(&sem->lock);
  printf("(%s = %d)\n", sem->name, sem->value);
    Assert(strcmp(fence, "FUCK", 4) == 0, "FENCE BOOM");
  while (sem->value <= 0) {
    Assert(strcmp(fence, "FUCK", 4) == 0, "FENCE BOOM");
    printf("(%s = %d)", sem->name, sem->value);
    spinlock_release(&sem->lock);
    asm volatile ("int $0x80" : : "a"(SYS_sem_wait), "b"(sem));
    spinlock_acquire(&sem->lock);
  }
  --sem->value;
  spinlock_release(&sem->lock);
}

void semaphore_signal(struct semaphore *sem) {
  spinlock_acquire(&sem->lock);
  ++sem->value;
  asm volatile ("int $0x80" : : "a"(SYS_sem_signal), "b"(sem));
  spinlock_release(&sem->lock);
}
