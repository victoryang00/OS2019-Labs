#include "crepl.h"

int cnt = 0;
char input[2048] = "";
char output[20] = "";

int main(int argc, char *argv[]) {
  for ( ; ; ++cnt) {
    printf(" in[%d]: ", cnt);
    sprintf(output, "out[%d]: ", cnt);

    if (getline(input, 2048, stdin) < 0) break;
    printf("%s\n", input); 
  }
}

void error() {
  printf("\33[0m" FG_RED "Compilation failed."
}

bool compile() {

}

int calculate() {

}
