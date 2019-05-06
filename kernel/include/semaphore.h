#ifndef __SEMAPHORE_H__
#define __SEMAPHORE_H__

#include <stdbool.h>
#include <debug.h>

/**
 * Semaphore for MT programming.
 */

struct semaphore {
  const char *name;
  volatile int value;
  struct spinlock lock;
};

void semaphore_init(struct semaphore *, const char *, int);
void semaphore_wait(struct semaphore *);
void semaphore_signal(struct semaphore *);

#endif
