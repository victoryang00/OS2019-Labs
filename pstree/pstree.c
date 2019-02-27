/**
 * TREE STRUCT OF PROCESSES
 *
 * systemd -> 1st child -> grandchild 1...
 *                |              |
 *                |        grandchild 2...
 *                |
 *            2nd child -> grandchild...
 *                |
 *            NULL (end)
 *
 * died parent      orphan child
 *      X       ->  (disappear)
 */

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <dirent.h>
#include <memory.h>
#include <stdbool.h>
#include <sys/types.h>

/* define option and pid struct (class) */
struct option {
  const char* name;
  const char* full_name;
  bool* target;
};
struct process {
  pid_t pid;
  pid_t ppid;
  char name[32]; // man 2 prctl -> maximum 16 bytes
  char state;    // man 7 proc  -> character type
  struct process *parent; // parent process
  struct process *child;  // child process
  struct process *next;   // next process (same level)
} rootProcess = {1, 0, "systemd", 'X', NULL, NULL};

/* 3 functionality option of the program */
static bool OP_SHOWPID = false;
static bool OP_NUMERIC = false;
static bool OP_VERSION = false;

/* ICS-PA style options array */
const struct option options[] = { 
  { "-p", "--show-pids",    &OP_SHOWPID }, 
  { "-n", "--numeric-sort", &OP_NUMERIC },
  { "-V", "--version",      &OP_VERSION }
};
const int NR_OPTIONS = (int) sizeof(options) / sizeof(struct option);

/* definition of functions */
int parseOptions(int, char*[]);
int printPSTree();
bool isNumber(char*);
void readProcess(char*, char*);
void printProcessPID(struct process*);
struct process* findProcess(pid_t, struct process*);
void addProcess(struct process*);
void printProcess(struct process*);
void printParentProcesses(struct process*);

/* main entry of the program */
int main(int argc, char *argv[]) {
  int ErrArgc = 0;
  if ((ErrArgc = parseOptions(argc, argv)) != 0) {
    /* option parse failed */
    fprintf(stderr, "Invalid option \"%s\". Aborted.\n", argv[ErrArgc]);
    return -1;
  } else {
    /* option parse OK */
    if (OP_VERSION) {
      printf("pstree v0.0.0 from MiniLabs of OSLab.\n"
          "By doowzs (Tianyun Zhang) [171860508].\n");
      return 0;
    } else {
      int result = printPSTree();
      return result;
    }
  }
}

int parseOptions(int argc, char *argv[]) {
  bool hasMatch = false;
  // skip the process name (argv[0])
  for (int i = 1; i < argc; ++i) {
    assert(argv[i]); // always true

    hasMatch = false;
    for (int op = 0; op < NR_OPTIONS; ++op) {
      if (!strcmp(argv[i], options[op].name)
          || !strcmp(argv[i], options[op].full_name)) {
        hasMatch = true;
        *(options[op].target) = true;
      }
    }
    if (!hasMatch) return i; // match failed
  }
  assert(!argv[argc]); // always true
  return 0;
}

int printPSTree() {
  DIR *dr = opendir("/proc");
  if (!dr) {
    fprintf(stderr, "Error opening /proc folder. Aborted.\n");
    return -1;
  }
  struct dirent *dp;
  while ((dp = readdir(dr)) != NULL) {
    if (isNumber(dp->d_name)) {
      /* read the process */
      readProcess(dp->d_name, NULL);
      /* read child threads */
      char taskFolder[64] = "/proc/";
      strncat(taskFolder, dp->d_name, 16);
      strcat(taskFolder, "/task");
      DIR *taskdr = opendir(taskFolder);
      if (taskdr) { // process may die at this moment
        struct dirent *childp;
        while ((childp = readdir(taskdr)) != NULL) {
          if (isNumber(childp->d_name)) readProcess(dp->d_name, childp->d_name);
        }
      }
    }
  }
  closedir(dr);

  if (OP_SHOWPID) printProcessPID(rootProcess);
  printProcess(&rootProcess);
  return 0;
}

bool isNumber(char *s) {
  int len = strlen(s);
  for (int i = 0; i < len; ++i) {
    if (!isdigit(s[i])) return false;
  }
  return true;
}

void readProcess(char* pidStr, char* taskPidStr) {
  char statFile[64] = "";

  if (!taskPidStr) {
    /* read a process */
    sprintf(statFile, "/proc/%s/stat", pidStr);
  } else {
    /* read a child thread */
    sprintf(statFile, "/proc/%s/task/%s/stat", pidStr, taskPidStr);
  }

  FILE* sfp = fopen(statFile, "r");
  if (sfp) { // process may die before this moment 
    struct process* proc = malloc(sizeof(struct process));
    fscanf(sfp, "%d (%s %c %d", &proc->pid, proc->name, &proc->state, &proc->ppid);
    proc->name[strlen(proc->name) - 1] = '\0';
    if (taskPidStr) {
      char name[32] = "";
      strcpy(name, proc->name);
      sprintf(proc->name, "{%.*s}", 16, name);
      proc->ppid = (pid_t) strtol(pidStr, NULL, 10); // for threads, use Tgid instead of Ppid.
    }
    if (OP_SHOWPID) printProcessPID(proc); 
    proc->parent = proc->child = proc->next = NULL;
    addProcess(proc);
  }  
}

void printProcessPID(struct process* proc) {
  char pidStr[32] = "";
  sprintf(pidStr, "(%d)", proc->pid);
  strncat(proc->name, pidStr, 14); // avoid overflow
} 

struct process* findProcess(pid_t pid, struct process* cur) {
  /* start from root if not given */
  if (!cur) cur = &rootProcess;

  /* end of recursion (found) */
  if (cur->pid == pid) return cur;

  /* start of next recursion (go deeper or parallel) */
  struct process* result = NULL;  
  if (cur->child) {
    result = findProcess(pid, cur->child);
    if (result) return result;
  }
  if (cur->next) {
    result = findProcess(pid, cur->next);
    if (result) return result;
  }
  return NULL; // not found
}

void addProcess(struct process* proc) {
  /* Avoid duplication. */
  struct process* self = findProcess(proc->pid, NULL);
  if (self) return;
  else {
    /* If the process is an orphan (parent is dead),
     * then it does not appear in the process tree. */
    struct process* parent = findProcess(proc->ppid, NULL);
    if (!parent) return;
    else {
      proc->parent = parent;
      struct process* child = parent->child;
      if (!child) {
        /* no child now */
        parent->child = proc;
      } else {
        if (OP_NUMERIC) {
          if (proc->pid < child->pid) {
            proc->next = child;
            parent->child = proc;
          } else {
            while (child->next != NULL && proc->pid > child->pid) child = child->next;
            proc->next = child->next;
            child->next = proc;
          }
        } else {
          proc->next = child;
          parent->child = proc;
        }
      }
    }
  }
}

void printProcess(struct process* proc) {
  printf("%s%s%s", 
      (proc == &rootProcess ? "" : (proc == proc->parent->child ? (proc->next ? "-+-" : "---") : (proc->next ? " |-" : " `-"))), 
      proc->name, 
      proc->child ? "" : "\n");
  
  if (proc->child) printProcess(proc->child);
  if (proc->next) {
    printParentProcesses(proc->next->parent);
    printProcess(proc->next);
  }
}

void printParentProcesses(struct process* proc) {
  /* Print the vertical lines of parent processes */
  if (proc->parent) printParentProcesses(proc->parent);
  printf("%s%*s",
      (proc == &rootProcess ? "" : (proc->next ? " | " : "   ")),
      (int) strlen(proc->name), "");
}
