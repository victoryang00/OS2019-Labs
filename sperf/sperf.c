#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>

#define DEBUG
#include "debug.h"

void sperf(int, char *[], char *[]);

int main(int argc, char *argv[], char *envp[]) {
  Assert(argc > 0, "Usage: sperf-32/64 cmd -arg1 -arg2 ...");
  sperf(argc, argv, envp);
  return 0;
}

void sperf(int argc, char *argv[], char *envp[]) {
  int cpid = 0;
  int pipefd[2] = {};
  char buf = 0;

  Assert(pipe(pipefd) != -1, "Pipe failed.");
  cpid = fork();
  Assert(cpid != -1, "Fork failed.");

  if (cpid == 0) {
    /* child process */
    close(pipefd[0]);
    dup2(pipefd[1], 1); // stdout
    dup2(pipefd[1], 2); // stderr
    Log("Child point A");
    int ret = execve(argv[0], argv, envp);
    Log("Child point B, ret = %d", ret);
    Panic("Should not return from execve!");
  } else {
    /* parent process */
    close(pipefd[1]);
    while (read(pipefd[0], &buf, 1) > 0) {
      printf("%c", buf);
      // TODO
    }
    close(pipefd[0]);
    wait(NULL);
  }
}
