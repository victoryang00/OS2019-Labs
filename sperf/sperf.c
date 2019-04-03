#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define DEBUG
#include "debug.h"

void sperf(int, char *[]);
void child(int, int, char *[]);
void parent(int);

int main(int argc, char *argv[]) {
  Assert(argc > 1, "Usage: sperf-32/64 cmd -arg1 -arg2 ...");
  sperf(argc, argv);
  return 0;
}

void sperf(int argc, char *argv[]) {
  int cpid = 0;
  int pipefd[2] = {};

  Assert(pipe(pipefd) != -1, "Pipe failed.");
  cpid = fork();
  Assert(cpid != -1, "Fork failed.");

  if (cpid == 0) {
    /* child process */
    close(pipefd[0]);
    child(pipefd[1], argc, argv);
    Panic("Should not return from child!");
  } else {
    /* parent process */
    parent(pipefd[0]);
    close(pipefd[0]);
    close(pipefd[1]);
    //wait(NULL);
  }
}

void child(int fd, int argc, char *argv[]) {
  char *real_argv[argc + 2];
  real_argv[0] = "strace";
  real_argv[1] = "-x";
  real_argv[2] = "-T";
  memcpy(real_argv + 3, argv + 1, (argc - 1) * sizeof(char *));

  // not execve because we need environmental variables
  int bh = open("/dev/null", O_APPEND);
  dup2(bh, 1); // stdout -> blackhole
  dup2(fd, 2); // stderr -> pipe
  execvp(real_argv[0], real_argv); 
  Panic("strace is not executable. (NO PATH HITS.)");
}

void parent(int fd) {
  char buf = 0;
  char line[1024] = "";
  int length = 0;

  char name[128] = "";
  double time = -1.0;
  
  while (read(fd, &buf, 1) != EOF) {
    line[length++] = buf;
    if (buf == '\n') {
      line[length] = 0;
      length = 0;
      
      Log("%s", line);
      if (name[0] == 0) {
        sscanf(line, "%[^(]%*[^<]<%lf>", name, &time);
      }
      if (name[0] != 0) {
        if (time < 0) {
          sscanf(line, "%*[^<]<%lf>", &time);
          if (time < 0) continue;
        }
        CLog(BG_GREEN, "%s %lf", name, time);
        //TODO: HANDLE
        name[0] = 0;
        time = -1.0;
      } 
    }
  }
}
