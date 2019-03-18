#ifndef __COMMON_H__

#include <kernel.h>
#include <nanos.h>

#define DEBUG
#ifdef DEBUG
#include <debug.h>
#endif

#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#endif
