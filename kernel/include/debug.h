#ifndef __DEBUG_H__
#define __DEBUG_H__

#define FG_BLACK  "\033[1;30m"
#define FG_RED    "\033[1;31m"
#define FG_GREEN  "\033[1;32m"
#define FG_YELLOW "\033[1;33m"
#define FG_BLUE   "\033[1;34m"
#define FG_PURPLE "\033[1;35m"
#define FG_CYAN   "\033[1;36m"
#define FG_WHITE  "\033[1;37m"

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
  printf("\33[0m" BG_BLUE "[%d][%s,%d,%s] " format " \33[0m\n", \
    _cpu(), __FILE__, __LINE__, __func__, ## __VA_ARGS__)
#define CLog(color, format, ...) \
  printf("\33[0m" color "[%d][%s,%d,%s] " format " \33[0m\n", \
    _cpu(), __FILE__, __LINE__, __func__, ## __VA_ARGS__)

#else
#define Log(format, ...) ;
#define CLog(color, format, ...) ;
#endif

#define Assert(cond, format, ...) \
  do { \
    if (!(cond)) { \
      printf("\33[0m" BG_RED "[%d][%s,%d,%s] " format " \33[0m\n", \
          _cpu(), __FILE__, __LINE__, __func__, ## __VA_ARGS__); \
      assert(cond); \
    } \
  } while (0)

#define Panic(format, ...) \
  printf("\33[0m" BG_RED "[%d][%s,%d,%s] " format " \33[0m\n", \
    _cpu(), __FILE__, __LINE__, __func__, ## __VA_ARGS__); \
  assert(0)

#endif
