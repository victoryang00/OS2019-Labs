#ifndef __DEBUG_H__
#define __DEBUG_H__

#ifdef DEBUG
#define Log(format, ...) \
  printf("\33[0m\33[1;44m[%d][%s,%d,%s] " format "\33[0m\n", \
    _cpu(), __FILE__, __LINE__, __func__, ## __VA_ARGS__)
#else
#define Log(format, ...) ;
#endif

#define Assert(cond, format, ...) \
  do { \
    if (!(cond)) { \
      printf("\33[0m\33[1;41m[%d][%s,%d,%s] " format "\33[0m\n", \
          _cpu(), __FILE__, __LINE__, __func__, ## __VA_ARGS__); \
      assert(cond); \
    } \
  } while (0)

#define Panic(format, ...) \
  printf("\33[0m\33[1;43m[%d][%s,%d,%s] " format "\33[0m\n", \
    _cpu(), __FILE__, __LINE__, __func__, ## __VA_ARGS__); \
  assert(0)

#endif
