#include <common.h>
#include <debug.h>
#include <thread.h>
#include <syscall.h>
#include <semaphore.h>

void syscall_ret(_Context *c, uintptr_t val) {
  c->GPRx = val;
}

_Context* do_syscall(_Context *context) {
  uintptr_t a[4] = {
    context->GPR1,
    context->GPR2,
    context->GPR3,
    context->GPR4
  };

  switch (a[0]) {
    case SYS_sleep:
      Assert(a[2] == &((struct semaphore *)a[1])->lock, "bad semaphore and lock");
      Assert(!spinlock_holding(((struct semaphore *) a[1]))->lock, "holding spinlock when to sleep");
      sys_sleep((void *) a[1], (struct spinlock *) a[2]);
      break;
    case SYS_wakeup:
      Assert(spinlock_holding(&((struct semaphore *) a[1]))->lock, "not holding spinlock when to wake up others");
      sys_wakeup((void *) a[1]);
      break;
    default: Panic("Unhandled syscall ID = %d", a[0]);
  }

  CLog(FG_YELLOW, "Syscall %d finished.", a[0]);
  return NULL;
}
