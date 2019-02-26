#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

struct option {
  char* name, full_name;
  bool* target;
};

static bool OP_P, OP_N, OP_V;

const struct option options[] = { 
  { "-p", "--show-pids",    &OP_P }, 
  { "-n", "--numeric-sort", &OP_N },
  { "-V", "--version",      &OP_V }
};

int parseOptions(int, char*[]);
void printPSTree();

int main(int argc, char *argv[]) {
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
