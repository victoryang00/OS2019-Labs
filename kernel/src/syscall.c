#include <common.h>
#include <debug.h>
#include <thread.h>
#include <syscall.h>

_Context* do_syscall(_Event ev, _Context *context) {
  Assert(ev.event == _EVENT_SYSCALL, "not a syscall");
  Assert(context->eax != -1, "bad eax for syscall");
  
  uintptr_t a[4];
  a[0] = context->GPR1;
  a[1] = context->ebx;
  a[2] = context->ecx;
  a[3] = context->edx;
  uintptr_t ret = 0;

  switch (a[0]) {
    case SYS_sem_wait:
      ret = kmt_sem_sleep((void *) a[1]);
      break;
    case SYS_sem_signal:
      ret = kmt_sem_wakeup((void *) a[1]);
      break;
    default: Panic("Unhandled syscall ID = %d", a[0]);
  }

  context->GPRx = ret;
  CLog(FG_YELLOW, "Syscall %d finished.", a[0]);

  return NULL;
}
