#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

int main(int argc, char *argv[]) {
  printf("Hello, World!\n");
  parseOptions(argc, argv);
  return 0;
}

void parseOptions(int argc, char *argv[]) {
  // skip the process name (argv[0])
  for (int i = 1; i < argc; ++i) {
    assert(argv[i]); // always true
    printf("argv[%d] = %s\n", i, argv[i]);
  }
  assert(!argv[argc]); // always true
}
