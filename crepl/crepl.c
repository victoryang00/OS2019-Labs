#include "crepl.h"

int io_count = 0;
size_t input_size = 0;
size_t buf_size = 0;
char *input = NULL;
char output[20] = "";
char func_name[128] = "";
int calc_result = 0;

int main(int argc, char *argv[]) {
  for ( ; ; ++io_count) {
    printf(" in[%d]: ", io_count);
    sprintf(output, "out[%d]: ", io_count);

    if ((input_size = getline((char **) &input, &buf_size, stdin)) <= 0) break;
    Log("%s", input);

    switch (precheck()) {
      case TYPE_FUNCT:
        if (compile(input, input_size)) {
          printf("%s", output); 
          printf("\33[0m" FG_GREEN "Loaded as function/variable %s.\n" "\033[0m", func_name);
        } else {
          printf("%s", output); 
          printf("\33[0m" FG_RED "Compilation error.\n" "\033[0m");
        }
        break;
      case TYPE_VALUE:
        if (calculate(input, input_size)) {
          printf("%s", output); 
          printf("\33[0m" FG_GREEN "Result: %d.\n" "\033[0m", calc_result);
        } else {
          printf("%s", output); 
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
  while (pos < input_size && isblank(input[pos]) == ' ') ++pos;
  if (pos >= input_size) return TYPE_INVAL;
  if (strncmp(input + pos, "int ", 4) == 0) {
    return TYPE_FUNCT;
  } else {
    return TYPE_VALUE;  
  }
}

bool compile(char *code, size_t size) {
  /* scan the name of the function */
  sscanf(code, " int %[^(]", func_name);
  CLog(BG_PURPLE, "The name of func is %s.", func_name);

  /* create temporaty files */
  char name_src[16] = "SRC_XXXXXX";
  char name_dst[16] = "DST_XXXXXX";
  char file_src[128] = "";
  char file_dst[128] = "";
  int fd_src = mkstemp(name_src);
  int fd_dst = mkstemp(name_dst);
  sprintf(file_src, "/proc/self/fd/%d", fd_src);
  sprintf(file_dst, "/proc/self/fd/%d", fd_dst);
  write(fd_src, code, size);
  CLog(BG_PURPLE, "Temporary files (%s, %s) created.", name_src, name_dst);

  char *CC_argv[] = {
    "gcc", 
    "-c",      // Compile and stop
    "-fPIC",   // Generate PI Code
    "-x", "c", // Language: ANSI C
    CC_ABI,    // ABI Type: 32, 64
    "-w",      // Inhibit warnings
    "-o", file_dst, file_src,
    NULL
  };
  CLog(BG_PURPLE, "GCC's target ABI is %s.", CC_ABI);

  pid_t pid = fork();
  Assert(pid >= 0, "Fork failed.");
  if (pid == 0) {
    /* fork a process to call gcc */
    execvp(CC_argv[0], CC_argv);
    Panic("execvp() shall not return!");
  } else {
    /* wait for the child process */
    int wstatus = 0;
    wait(&wstatus);
    if (WEXITSTATUS(wstatus) != 0) {
      CLog(BG_RED, "Child process exited with %d.", WEXITSTATUS(wstatus));
      return false;
    }
  }

  /* load the dynamic library */
  dlopen(file_dst, RTLD_GLOBAL);

  /* close temporaty files */
  close(fd_src);
  close(fd_dst);

  return true;
}

bool calculate(char *code, size_t size) {
  code = func_wrapper(code, &size);
  printf("NOT IMPLEMENTED!\n");
  free(code);
  return true;
}

char *func_wrapper(char *code, size_t *size) {
  char *ret = malloc(sizeof(char) * (*size + 64));
  sprintf(ret, "int __EVAL_FUNC_%d() { %s }", io_count, code);
  *size = strlen(ret);
  return ret;
}
