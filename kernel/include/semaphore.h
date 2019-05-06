#ifndef __SEMAPHORE_H__
#define __SEMAPHORE_H__

#include <stdbool.h>
#include <debug.h>

/**
 * Semaphore for MT programming.
 */

struct semaphore {
  const char fence[5] = { 'F', 'U', 'C', 'K', '\0' };
  const char *name;
  int value;
  struct spinlock lock;
};

void semaphore_init(struct semaphore *, const char *, int);
void semaphore_wait(struct semaphore *);
void semaphore_signal(struct semaphore *);

#endif
