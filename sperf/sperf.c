#include "sperf.h"

double time_total = 0;
perf_item *root = NULL;

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
  int bh = open("/dev/null", 0);
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
      
      //Log("%s", line);
      if (name[0] == 0) {
        sscanf(line, "%[^(]%*[^<]<%lf>", name, &time);
      }
      if (name[0] != 0) {
        if (time < 0) {
          sscanf(line, "%*[^<]<%lf>", &time);
          if (time < 0) continue;
        }
        CLog(BG_GREEN, "%s %lf", name, time);
        addItem(name, time);
        //TODO: showItems();
        name[0] = 0;
        time = -1.0;
      } 
    }
  }
}

void addItem(char *name, double time) {
  time_total += time;

  perf_item *pp = root;
  while (pp && strncmp(pp->name, name, SZ_NAME - 1) != 0) pp = pp->next;
  if (pp) {
    pp->time += time;
    /* disconnect pp from chain */
    if (pp == root) {
      root = pp->next;
    } else {
      perf_item *np = root;
      while (np->next && np->next != pp) np = np->next;
      np->next = pp->next;
    }
  } else {
    /* create a new perf node item */
    pp = malloc(sizeof(perf_item));
    strncpy(pp->name, name, SZ_NAME - 1);
    pp->time = time;
  }

  /* rejoin the chain to sort */
  if (!root || root->time < pp->time) {
    pp->next = root;
    root = pp;
  } else {
    perf_item *np = root;
    while (np->next && np->next->time > pp->time) np = np->next;
    pp->next = np->next;
    np->next = pp;
  }
}

void showItems() {
  perf_item *pp = root;
  for (; pp != NULL; pp = pp->next) {
    printf("%s : %.5lxs (%d\%)", pp->name, pp->time, (int) (pp->time / time_total * 100));
  }
}
