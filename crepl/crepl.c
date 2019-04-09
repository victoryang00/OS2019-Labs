#include "crepl.h"

int cnt = 0;
size_t len = 0;
ssize_t nread = 0;
char *input = NULL;
char output[20] = "";

int main(int argc, char *argv[]) {
  for ( ; ; ++cnt) {
    printf(" in[%d]: ", cnt);
    sprintf(output, "out[%d]: ", cnt);

    if (getline(&input, &len, stdin) < 0) break;
    printf("%s%s\n", output, input); 
  }
}

void error() {
  printf("\33[0m" FG_RED "Compilation failed." "\33[0m");
}

bool compile() {
  // TODO
  return true;
}

int calculate() {
  // TODO
  return 0;
}
