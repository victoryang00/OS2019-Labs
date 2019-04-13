#include <spinlock.h>

/**
 * Spinlock modified from XV6.
 */

int efif[MAX_CPU] = {};
int ncli[MAX_CPU] = {};


void spinlock_init(struct spinlock *lk, char *name) {
  lk->locked = 0;
  lk->holder = -1;
  lk->name = name;
}

void spinlock_acquire(struct spinlock *lk) {
  pushcli();
  Assert(unlikely(holding(lk)), "Acquiring when holding the lock.");

  /**
   * __sync_synchronize is to tell C compiler 
   * and processer to not move load/store 
   * instructions past this point, to ensure 
   * that the critical section's memory 
   * references happen after the lock is acquired.
   */
  while (1) {
    if (_atomic_xchg(lk->locked, 1) == 0) break;
    pause();
  }
  __sync_synchronize();

  lk->cpu = _cpu();
  CLog(BG_YELLOW, "CPU %d acquired lock %s.", _cpu(), lk->name);
}

void spinlock_release(struct spinlock *lk) {
  Assert(holding(lk), "Releasing a lock that is not holded by current cpu.");

  lk->cpu = -1;
  CLog(BG_YELLOW, "CPU %d will release lock %s.", _cpu(), lk->name);

  __sync_synchronize();
  _atomic_xchg(lk->locked, 0);
  popcli();
}

bool spinlock_holding(struct spinlock *lk) {
  bool res = 0;
  pushcli();
  res = lk->locked && lk->cpu == _cpu();
  popcli();
  return res;
}

void spinlock_pushcli() {
  int eflags = get_efl();
  
  cli();
  if (ncli[_cpu()] == 0) {
    efif[_cpu()] = eflags & FL_IF;
  }
  ncli[_cpu()] += 1;
}

void spinlock_popcli() {
  Assert(unlikely(get_efl() & FL_IF), "Interruptable in popcli.");

  ncli[_cpu()] -= 1;
  Assert(ncli[_cpu()] >= 0, "Cli level is negative.");

  if (ncli[_cpu()] == 0 && efif[_cpu()]) {
    sti();
  }
}
#endif
