#ifndef __LOCK_H__
#define __LOCK_H__

#include <debug.h>

void lock(volatile int *exclusion) {
  CLog(BG_YELLOW, "LOCK ON");
  while (__sync_lock_test_and_set(exclusion, 1)) {
    while (*exclusion) {
      ;
    }
  }
}

void unlock(volatile int *exclusion) {
  CLog(BG_YELLOW, "LOCK OFF");
  __sync_synchronize();
  *exclusion = 0;
}

#endif
