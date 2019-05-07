#ifndef __SEMAPHORE_H__
#define __SEMAPHORE_H__

#include <stdbool.h>
#include <debug.h>

/**
 * Semaphore for MT programming.
 */

struct alarm_log {
  void *alarm;
  struct alarm_log *next;
};

struct semaphore {
  struct spinlock lock;
  const char *name;
  volatile int value;
};

void semaphore_init(struct semaphore *, const char *, int);
void semaphore_wait(struct semaphore *);
void semaphore_signal(struct semaphore *);

#endif
