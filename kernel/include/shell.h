#ifndef __SHELL_H__
#define __SHELL_H__

#include <common.h>

typedef struct cmd {
  const char *name;
  void (*func)(char *arg, char *ret);
} cmd_t;

void shell_task(void *arg);
int handle_command(char *cmd);

void ping(char *arg, char *ret);
void fuck(char *arg, char *ret);
void echo(char *arg, char *ret);

void ls(char *arg, char *ret);
void cd(char *arg, char *ret);
void cat(char *arg, char *ret);
void mkdir(char *arg, char *ret);
void rmdir(char *arg, char *ret);
void rm(char *arg, char *ret);

#endif
