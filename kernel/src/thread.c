#include <common.h>
#include <spinlock.h>
#include <semaphore.h>

/**
 * Kernel Multi-Thread module (KMT, Proc)
 */

struct spinlock proc_lock;
struct task root_task;

void kmt_init() {
  spinlock_init(&proc_lock, "KMT(Proc) Lock");
}

int kmt_create(struct task *task, const char *name, void (*entry)(void *arg), void *arg) {

}

void kmt_teardown(struct task *task) {

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
}
