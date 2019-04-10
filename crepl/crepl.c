#include "crepl.h"

int io_count = 0;
ssize_t input_size = 0;
size_t buf_size = 0;
char *input = NULL;
char output[20] = "";
char func_name[128] = "";
int calc_result = 0;

int main(int argc, char *argv[]) {
  for (io_count = 1; ; ++io_count) {
    printf(" in[%d]: ", io_count);
    sprintf(output, "out[%d]: ", io_count);

    if ((input_size = getline((char **) &input, &buf_size, stdin)) <= 0) break;
    while (isspace(input[input_size - 1])) --input_size;
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
  while (pos < input_size && isspace(input[pos])) ++pos;
  if (pos >= input_size) return TYPE_INVAL;
  if (strncmp(input + pos, "int ", 4) == 0) {
    return TYPE_FUNCT;
  } else {
    return TYPE_VALUE;  
  }
}

bool compile(char *code, size_t size) {
  /* scan the name of the function */
  sscanf(code, " int %[^ (]", func_name);
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
    "-shared", // Shared library
    "-fPIC",   // Generate PI Code
    "-x", "c", // Language: ANSI C
    CC_ABI,    // ABI Type: 32, 64
    "-Werror", // Block all warnings
    "-o", file_dst, file_src,
    NULL
  };
  CLog(BG_PURPLE, "GCC's target ABI is %s.", CC_ABI);

  bool success = false;
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
    } else {
      if (dlopen(file_dst, RTLD_GLOBAL) == NULL) {
        CLog(BG_RED, "Failed to load dynamic library (dlopen).");
      } else {
        CLog(BG_GREEN, "Dynamic library %s loaded.", file_dst);
        success = true;
      }
    }
  }
  close(fd_src);
  close(fd_dst);
  return success;
}

bool calculate(char *code, size_t size) {
  code = func_wrapper(code, &size);
  bool cc_success = compile(code, size);
  free(code);
  if (!cc_success) {
    return false;
  } else {
    calc_result = 12450;
    return true;
  }
}

char *func_wrapper(char *code, size_t *size) {
  char *ret = malloc(sizeof(char) * (*size + 64));
  sprintf(ret, "int " FUNC_PREFIX "%d() {\n  return (%s);\n}", io_count, code);
  *size = strlen(ret);
  return ret;
}
