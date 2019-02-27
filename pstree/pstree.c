/**
 * For teachers and other readers:
 * 
 * HOW THIS $HIT CODE WORKS:
 * 0. Workflow:
 *    main() -> parseOptions() -> printPSTree()
 * 1. I used a ICS-PA style array to parse all 
 *    options in an iteration.
 * 2. Read the /proc directory and open numeric
 *    folders one by one.
 * 3. When reading one process, read all its
 *    threads and add them into the tree. If -n
 *    option is given, then sort the tree when
 *    inserting the process nodes.
 * 4. The tree is constructed by the following
 *    linked data structure.
 * 5. Perform a recursive algorithm to print
 *    the tree to screen in a neat way.
 * 6. If -p option is given, then print the PID
 *    of processes to proc->name array when 
 *    printing the process tree.
 *
 * +------------------------------------------+
 * | DATA STRUCTURE OF A PROCESS              |
 * |                                          |
 * | struct process                           |
 * |  |- pid:    the id of this process       |
 * |  |- ppid:   the id of parent process     |
 * |  |          (ppid or tgid for threads)   |
 * |  |- name:   the name of process          |
 * |  |- state:  not used in my program       |
 * |  |- parent: pointer to parent node       |
 * |  |- child:  pointer to first child       |
 * |  `- next:   pointer to next brother      |
 * +------------------------------------------+
 * | TREE STRUCTURE OF PROCESSES              |
 * |                                          |
 * | systemd -> 1st child -> grandchild 1...  |
 * |   (1)          |              |          |
 * |                |        grandchild 2...  |
 * |                |                         |
 * |            2nd child -> grandchild...    |
 * |                |                         |
 * |            NULL (end)                    |
 * |                                          |
 * | died parent      orphan child            |
 * |      X       ->  (disappear in tree)     |
 * +------------------------------------------+
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

/* 3 functionality options of the program */
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
struct process* readProcess(char*, struct process*);
void attachProcessPID(struct process*);
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
    if (isdigit(*(dp->d_name))) {
      /* read the process */
      struct process* parent = readProcess(dp->d_name, NULL);
      if (!parent) continue;
      /* read child threads */
      char taskFolder[64] = "/proc/";
      sprintf(taskFolder, "/proc/%.16s/task", dp->d_name);
      DIR *taskdr = opendir(taskFolder);
      if (taskdr) { // process may die at this moment
        struct dirent *childp;
        while ((childp = readdir(taskdr)) != NULL) {
          if (isdigit(*(childp->d_name))) readProcess(childp->d_name, parent);
        }
      }
    }
  }
  closedir(dr);

  printProcess(&rootProcess); // print pstree
  return 0;
}

struct process* readProcess(char* pidStr, struct process* parent) {
  char statFile[64] = "";

  if (!parent) {
    /* read a process */
    sprintf(statFile, "/proc/%.12s/stat", pidStr);
  } else {
    /* read a child thread */
    sprintf(statFile, "/proc/%d/task/%.12s/stat", parent->pid, pidStr);
  }

  FILE* sfp = fopen(statFile, "r");
  if (sfp) { // process may die before this moment 
    struct process* proc = malloc(sizeof(struct process));
    fscanf(sfp, "%d (%16[^)] %c %d", &proc->pid, proc->name, &proc->state, &proc->ppid);
    proc->parent = proc->child = proc->next = NULL;
    if (parent) {
      proc->ppid = parent->pid; 
      sprintf(proc->name, "{%.16s}", parent->name);
    }
    addProcess(proc);
    fclose(sfp); // I forgot to close the stream and caused bug.
    return proc;
  } else {
    return NULL;
  }
}

void attachProcessPID(struct process* proc) {
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
    if (parent) {
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
            while (child->next && proc->pid > child->next->pid) child = child->next;
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
  /* print (pid) to name */
  if (OP_SHOWPID) attachProcessPID(proc);

  printf("%s%s%s", 
      (proc == &rootProcess ? "" : (proc == proc->parent->child ? (proc->next ? "-+-" : "---") : (proc->next ? " |-" : " `-"))), 
      proc->name, 
      proc->child ? "" : "\n");
  
  if (proc->child) printProcess(proc->child);
  if (proc->next) {
    if (proc->next->parent) printParentProcesses(proc->next->parent);
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
