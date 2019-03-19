#ifndef __LOCK_H__
#define __LOCK_H__

void lock(volatile int *exclusion) {
  while (__sync_lock_test_and_set(exclusion, 1)) {
    while (*exclusion) {
      ;
    }
  }
}

void unlock(volatile int *exclusion) {
  __sync_synchronize();
  *exclusion = 0;
}

#endif
