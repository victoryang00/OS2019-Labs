#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

/* define option struct (class) */
struct option {
  const char* name;
  const char* full_name;
  bool* target;
};

/* 3 functionality option of the program */
static bool OP_P, OP_N, OP_V;

/* ICS-PA style options array */
const struct option options[] = { 
  { "-p", "--show-pids",    &OP_P }, 
  { "-n", "--numeric-sort", &OP_N },
  { "-V", "--version",      &OP_V }
};
#define NR_OPTIONS sizeof(options) / sizeof(struct option)

/* definition of functions */
int parseOptions(int, char*[]);
void printPSTree();

/* main entry of the program */
int main(int argc, char *argv[]) {
  printf("sizeof options is %d\n", NR_OPTIONS);
  int ErrArgc = 0;
  if ((ErrArgc = parseOptions(argc, argv)) != 0) {
    // option parse failed
    fprintf(stderr, "Invalid option \"%s\". Aborted.\n", argv[ErrArgc]);
    return -1;
  } else {
    // option parse OK
    printPSTree();
  }
  return 0;
}

int parseOptions(int argc, char *argv[]) {
  // skip the process name (argv[0])
  for (int i = 1; i < argc; ++i) {
    assert(argv[i]); // always true
    printf("argv[%d] = %s\n", i, argv[i]);
    // TODO: parse detailed functions.
  }
  assert(!argv[argc]); // always true
  return 0;
}

void printPSTree() {
  //TODO: implement the function
}
