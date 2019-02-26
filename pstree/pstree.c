#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

int parseOptions(int, char*[]);

int main(int argc, char *argv[]) {
  int NR_errop = 0;
  if ((NR_errop = parseOptions(argc, argv)) != 0) {
    fprintf(stderr, "Invalid option \"%s\". Aborted.", argv[NR_errop]);
    return -1;
  } else {
    // parse OK
  }
  return 0;
}

int parseOptions(int argc, char *argv[]) {
  // skip the process name (argv[0])
  for (int i = 1; i < argc; ++i) {
    assert(argv[i]); // always true
    printf("argv[%d] = %s\n", i, argv[i]);
  }
  assert(!argv[argc]); // always true
  return 0;
}
