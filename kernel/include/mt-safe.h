#ifndef __MT_SAFE_H__
#define __MT_SAFE_H__

#include <common.h>
#include <debug.h>
#include <spinlock.h>

/**
 * MT-SAFE Wrapper: wrap the functions that is not MT-SAFE so that program
 * will be blocked if another process is inside the function.
 */

// ----------------------------------------------------------------------------
// printf: lock defined in src/os.c.

extern struct spinlock printf_lock;
#define printf(...) \
  __sync_synchronize(); \
  spinlock_acquire(&printf_lock); \
  printf(__VA_ARGS__); \
  spinlock_release(&printf_lock); \
  __sync_synchronize()


#endif
