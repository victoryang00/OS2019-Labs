#include <common.h>
#include <debug.h>
#include <thread.h>
#include <syscall.h>

_Context* do_syscall(_Event ev, _Context *context) {
  Assert(ev.event == _EVENT_SYSCALL, "not a syscall");
  Assert(context->GPR1 != -1, "bad eax for syscall");
  
  uintptr_t a[4] = {};
  a[0] = context->GPR1;
  a[1] = context->GPR2;
  a[2] = context->GPR3;
  a[3] = context->GPR4;
  uintptr_t ret = 0;

  switch (a[0]) {
    case SYS_sleep:
      CLog(FG_YELLOW, "goto sleep");
      ret = kmt_sleep((void *) a[1], (struct spinlock *) a[2]);
      CLog(FG_YELLOW, "already sleep");
      break;
    case SYS_wakeup:
      CLog(FG_YELLOW, "goto wakeup");
      ret = kmt_wakeup((void *) a[1]);
      CLog(FG_YELLOW, "already wakeup");
      break;
    default: Panic("Unhandled syscall ID = %d", a[0]);
  }

  context->GPRx = ret;
  CLog(FG_YELLOW, "Syscall %d finished.", a[0]);

  return NULL;
}
