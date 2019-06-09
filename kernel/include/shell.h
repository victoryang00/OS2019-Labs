#ifndef __SHELL_H__
#define __SHELL_H__

#include <common.h>

#define FUNC(name) void name(const char *arg, char *pwd, char *ret)

typedef struct cmd {
  const char *name;
  void (*func)(const char *arg, char *pwd, char *ret);
} cmd_t;

void shell_task(void *arg);
bool get_dir(const char *arg, const char *pwd, char *dir);

FUNC(help);
FUNC(ping);
FUNC(fuck);
FUNC(echo);
FUNC(pwd);
FUNC(ls);
FUNC(cd);
FUNC(cat);
FUNC(write);
FUNC(append);
FUNC(link);
FUNC(mkdir);
FUNC(rmdir);
FUNC(rm);

#endif
