/**
 * TREE STRUCT OF PROCESSES
 *
 * ROOT -> 1st child -> grandchild...
 *             |
 *         2nd child -> grandchild...
 *             |
 *         NULL (end)
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
  struct process *child; // child process
  struct process *next;  // next process (same level)
} rootProcess = {0, 0, "root", 'X', NULL, NULL};

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
void readProcess(char*);
struct process* findProcess(pid_t, struct process*);
void addProcess(struct process*);

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
  if (dr == NULL) {
    fprintf(stderr, "Error opening /proc folder. Aborted.\n");
    return -1;
  }
  struct dirent *dp;
  while ((dp = readdir(dr)) != NULL) {
    if (isNumber(dp->d_name)) {
      printf("%s\n", dp->d_name);
      readProcess(dp->d_name);
    }  
  }
  closedir(dr);

  // TODO: USE RECURSIVE METHOD TO PRINT THE TREE.

  return 0;
}

bool isNumber(char *s) {
  int len = strlen(s);
  for (int i = 0; i < len; ++i) {
    if (!isdigit(s[i])) return false;
  }
  return true;
}

void readProcess(char* pidStr) {
  char statFile[64] = "";
  struct process* proc = malloc(sizeof(struct process));

  sprintf(statFile, "/proc/%s/stat", pidStr);
  FILE* sfp = fopen(statFile, "r");
  fscanf(sfp, "%d (%s) %c %d", &proc->pid, proc->name, &proc->state, &proc->ppid);

  addProcess(proc);
}

struct process* findProcess(pid_t pid, struct process* cur) {
  /* end of recursion (found) */
  if (cur->pid == pid) return cur;

  /* start from root if not given */
  if (!cur) cur = &rootProcess;
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
  struct process* parent = findProcess(proc->ppid, NULL);
  proc->next = parent->child;
  parent->child = proc;
}
