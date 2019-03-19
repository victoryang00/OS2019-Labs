#ifndef __DEBUG_H__
#define __DEBUG_H__

#define BG_BLACK  "\033[1;40m"
#define BG_RED    "\033[1;41m"
#define BG_GREEN  "\033[1;42m"
#define BG_YELLOW "\033[1;43m"
#define BG_BLUE   "\033[1;44m"
#define BG_PURPLE "\033[1;45m"
#define BG_CYAN   "\033[1;46m"
#define BG_WHITE  "\033[1;47m"

#ifdef DEBUG
#define Log(format, ...) \
  printf("\33[0m\33[1;44m[%d][%s,%d,%s] " format "\33[0m\n", \
    _cpu(), __FILE__, __LINE__, __func__, ## __VA_ARGS__)
#define CLog(color, format, ...) \
  printf("\33[0m" color "[%d][%s,%d,%s] " format "\33[0m\n", \
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
