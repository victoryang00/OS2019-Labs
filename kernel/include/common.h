#ifndef __COMMON_H__
#define __COMMON_H__

#include <am.h>
#include <x86.h>
#include <klib.h>
#include <limits.h>
#include <stdbool.h>

#include <kernel.h>
#include <nanos.h>

// defined in x86-nemu.h
#define MAX_CPU 8

//#define DEBUG
//#define MEM_DEBUG
//#define LOCK_DEBUG
#include <mt-safe.h>
#include <debug.h>

#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

static inline size_t power2ify(size_t x) {
  assert((int) x > 0);
  size_t ret = 1;
  while (ret < x) ret <<= 1;
  return ret;
}

#endif
