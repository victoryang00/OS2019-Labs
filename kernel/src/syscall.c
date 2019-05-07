#include <common.h>
#include <debug.h>
#include <thread.h>
#include <syscall.h>

void syscall_ret(_Context *c, uintptr_t val) {
  c->GPRx = val;
}

_Context* do_syscall(_Context *context) {
  Assert(ev.event == _EVENT_SYSCALL, "not a syscall");
  Assert(context->GPR1 != -1, "bad eax for syscall");
  
  uintptr_t a[4] = {
    context->GPR1,
    context->GPR2,
    context->GPR3,
    context->GPR4
  };

  switch (a[0]) {
    case SYS_sleep:
      sys_sleep((void *) a[1], (struct spinlock *) a[2]);
      break;
    case SYS_wakeup:
      sys_wakeup((void *) a[1]);
      break;
    default: Panic("Unhandled syscall ID = %d", a[0]);
  }

  CLog(FG_YELLOW, "Syscall %d finished.", a[0]);
  return NULL;
}
