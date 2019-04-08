#include "sperf.h"

int max_name_length = 0;
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

  char call_name[128] = "";
  double call_time = -1.0;
  
  int wstatus = 0;
  time_t next_frame = time(NULL) + TM_FRAME;
  
  while (
      waitpid(-1, &wstatus, WNOHANG) == 0
      && read(fd, &buf, 1) > 0
  ) {
    line[length++] = buf;
    if (buf == '\n') {
      line[length] = 0;
      length = 0;
      
      //Log("%s", line);
      if (call_name[0] == 0) {
        sscanf(line, "%[^(]%*[^<]<%lf>", call_name, &call_time);
      }
      if (call_name[0] != 0) {
        if (call_time < 0) {
          sscanf(line, "%*[^<]<%lf>", &call_time);
          if (call_time < 0) continue;
        }
        //CLog(BG_GREEN, "%s %lf", call_name, call_time);
        addItem(call_name, call_time);
        call_name[0] = 0;
        call_time = -1.0;
      } 
    }
    if (time(NULL) > next_frame) {
      next_frame += TM_FRAME;
      showItems();
    }
  }
  showItems();
  CLog(BG_GREEN, "The process has finished.");
}

void addItem(char *call_name, double call_time) {
  time_total += call_time;

  perf_item *pp = root;
  while (pp && strncmp(pp->call_name, call_name, SZ_NAME - 1) != 0) pp = pp->next;
  if (pp) {
    pp->call_time += call_time;
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
    if (strlen(call_name) > max_name_length) {
      max_name_length = strlen(call_name);
    }
    strncpy(pp->call_name, call_name, SZ_NAME - 1);
    pp->call_time = call_time;
  }

  /* rejoin the chain to sort */
  if (!root || root->call_time < pp->call_time) {
    pp->next = root;
    root = pp;
  } else {
    perf_item *np = root;
    while (np->next && np->next->call_time > pp->call_time) np = np->next;
    pp->next = np->next;
    np->next = pp;
  }
}

void showItems() {
  printf("\033[2J\033[1;1H");
  perf_item *pp = root;
  for (; pp != NULL; pp = pp->next) {
    printf("%*s : %.5lfs (%2d%%)\n", max_name_length, pp->call_name, pp->call_time, (int) (pp->call_time / time_total * 100));
  }
}
