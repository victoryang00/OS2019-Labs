#ifndef __DEBUG_H__
#define __DEBUG_H__

#ifdef DEBUG
#define Log(format, ...) \
    printf("\33[0m[\33[1;35mLog\33[0m]\33[1;34m[%s,%d,%s] " format "\33[0m\n", \
        __FILE__, __LINE__, __func__, ## __VA_ARGS__)

#define Assert(cond, format, ...) \
  do { \
    if (!(cond)) { \
      fflush(stdout); \
      Log("\033[31m[ASSERT]\033[0m" format, __VA_ARGS__); \
      assert(cond); \
    } \
  } while (0)
#endif

#endif
