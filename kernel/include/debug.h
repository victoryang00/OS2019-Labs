#ifndef __DEBUG_H__
#define __DEBUG_H__

enum BG_COLOR {
  black  = 40,
  red    = 41,
  green  = 42,
  yellow = 43,
  blue   = 44,
  purple = 45,
  cyan   = 46,
  white  = 47
};

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
