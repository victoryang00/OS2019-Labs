#ifndef __LOCK_H__
#define __LOCK_H__

#include <stdbool.h>
#include <debug.h>

/**
 * Spinlock modified from XV6.
 */

struct spinlock {
  const char *name;
  bool locked;
  int holder;
};

void spinlock_init(struct spinlock *, const char *);
void spinlock_acquire(struct spinlock *);
void spinlock_release(struct spinlock *);
bool spinlock_holding(struct spinlock *);
void spinlock_pushcli();
void spinlock_popcli();

#endif
