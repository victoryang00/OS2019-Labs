#include <unistd.h>
#include <stdbool.h>
#include "debug.h"

void sperf(int, char *[], char *[]);

int main(int argc, char *argv[], char *envp[]) {
  sperf(argc, argv, envp);
  return 0;
}

void sperf(int argc, char *argv[], char *envp[]) {
  int cpid = 0;
  int pipefd[2] = {};
  char buf[1024] = "";

  Assert(pipe(pipefd) != -1, "Pipe failed.");
  cpid = fork();
  Assert(cpid != -1, "Fork failed.");

  if (cpid == 0) {
    /* child process */
    close(pipefd[0]);
    dup2(pipefd[1], 1); // stdout
    dup2(pipefd[1], 2); // stderr
    execve(argv[0], argv + 1, envp);
    Assert(false, "Should not return from execve!");
  } else {
    /* parent process */
    close(pipefd[1]);
    while (read(pipefd[0], &buf, 1) > 0) {
      Log("%s\n", buf);
      // TODO
    }
    close(pipefd[0]);
    wait(NULL);
  }
}
