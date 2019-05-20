#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <stdio.h>
#include <assert.h>

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
  printf("\33[0m" BG_BLUE "[%s,%d,%s] " format " \33[0m\n", \
    __FILE__, __LINE__, __func__, ## __VA_ARGS__); \
  fflush(stdout) 
#define CLog(color, format, ...) \
  printf("\33[0m" color "[%s,%d,%s] " format " \33[0m\n", \
    __FILE__, __LINE__, __func__, ## __VA_ARGS__); \
  fflush(stdout)

#else
#define Log(format, ...) ;
#define CLog(color, format, ...) ;
#endif

#define Assert(cond, format, ...) \
  do { \
    if (!(cond)) { \
      CLog(BG_RED, format, ## __VA_ARGS__); \
      CLog(BG_RED, "Last errno was %d (%s)", errno, strerror(errno)); \
      assert(cond); \
      errno = 0; \
    } \
  } while (0)

#define Panic(format, ...) \
  CLog(BG_RED, format, ## __VA_ARGS__); \
  assert(0)

#endif
