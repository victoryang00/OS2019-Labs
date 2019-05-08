#include <common.h>
#include <spinlock.h>

/**
 * Spinlock modified from XV6.
 */

int efif[MAX_CPU] = {};
int ncli[MAX_CPU] = {};

void spinlock_init(struct spinlock *lk, const char *name) {
  lk->locked = 0;
  lk->holder = -1;
  lk->name = name;
}

void spinlock_acquire(struct spinlock *lk) {
  spinlock_pushcli();
  /**
   * __sync_synchronize is to tell C compiler 
   * and processer to not move load/store 
   * instructions past this point, to ensure 
   * that the critical section's memory 
   * references happen after the lock is acquired.
   */
  while (1) {
    if (_atomic_xchg((intptr_t *) &lk->locked, 1) == 0) break;
    pause();
  }
  __sync_synchronize();

  lk->holder = _cpu();
}

void spinlock_release(struct spinlock *lk) {
  lk->holder = -1;

  __sync_synchronize();
  _atomic_xchg((intptr_t *) &lk->locked, 0);
  spinlock_popcli();
}

bool spinlock_holding(struct spinlock *lk) {
  bool res = 0;
  spinlock_pushcli();
  res = lk->locked && lk->holder == _cpu();
  spinlock_popcli();
  return res;
}

void spinlock_pushcli() {
  int eflags = get_efl();
  
  cli();
  Assert((get_efl() & FL_IF) == 0, "cli() failed to turn off interrupt.");
  if (ncli[_cpu()] == 0) {
    efif[_cpu()] = eflags & FL_IF;
  }
  ncli[_cpu()] += 1;
}

void spinlock_popcli() {
  Assert((get_efl() & FL_IF) == 0, "Interruptable in popcli.");

  ncli[_cpu()] -= 1;
  Assert(ncli[_cpu()] >= 0, "Cli level is negative.");

  if (ncli[_cpu()] == 0 && efif[_cpu()]) {
    sti();
  }
}
