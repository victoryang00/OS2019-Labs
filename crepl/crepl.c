#include "crepl.h"

int cnt = 0, type = 0;
size_t len = 0;
ssize_t nread = 0;
char *input = NULL;
char output[20] = "";
char func_name[128] = "";
int calc_result = 0;

int main(int argc, char *argv[]) {
  for ( ; ; ++cnt) {
    printf(" in[%d]: ", cnt);
    sprintf(output, "out[%d]: ", cnt);

    if (getline(&input, &len, stdin) < 0) break;
    Log("%s", input);
    printf("%s", output); 

    switch (precheck()) {
      case TYPE_FUNCT:
        if (compile()) {
          printf("\33[0m" FG_GREEN "Added as function %s.\n" "\033[0m", func_name);
        } else {
          printf("\33[0m" FG_RED "Compilation error.\n" "\033[0m");
        }
        break;
      case TYPE_VALUE:
        if (calculate()) {
          printf("\33[0m" FG_GREEN "Result: %d.\n" "\033[0m", calc_result);
        } else {
          printf("\33[0m" FG_RED "Calculation error.\n" "\033[0m");
        }
        break;
      default:
        printf("\33[0m" FG_RED "Input validation failed.\n" "\33[0m");
        break;
    }
    printf("\n");
  }
}

int precheck() {
  size_t pos = 0;
  while (pos < len && isblank(input[pos]) == ' ') ++pos;
  if (pos >= len) return TYPE_INVAL;
  if (strncmp(input + pos, "int ", 4) == 0) {
    return TYPE_FUNCT;
  } else {
    return TYPE_VALUE;  
  }
}

bool compile() {
  // TODO: USE MKSTEMP OR TMPFILE???
  FILE *src = tmpfile();
  FILE *dst = tmpfile();
  fprintf(tmp, "%s", input);

  char *CC_argv[] = {
    "gcc", "-fPIC", 
    "-o", ""
  }
  execvp(CC_argv[0], CC_argv);
  
  return true;
}

bool calculate() {
  // TODO
  return true;
}
