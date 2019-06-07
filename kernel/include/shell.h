#ifndef __SHELL_H__
#define __SHELL_H__

#include <common.h>

#define FUNC(name) void name(const char *arg, char *pwd, char *ret)

typedef struct cmd {
  const char *name;
  void (*func)(const char *arg, char *pwd, char *ret);
} cmd_t;

void shell_task(void *arg);
int handle_command(char *cmd);

FUNC(help);
FUNC(ping);
FUNC(fuck);
FUNC(echo);
FUNC(ls);
FUNC(cd);
FUNC(cat);
FUNC(mkdir);
FUNC(rmdir);
FUNC(rm);

#endif
