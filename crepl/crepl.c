#include "crepl.h"

int io_count = 0;
ssize_t input_size = 0;
size_t buf_size = 0;
char *input = NULL;
char output[20] = "";

void *lib_handle = NULL;
char func_name[128] = "";
int calc_result = 0;

int main(int argc, char *argv[]) {
  printf("Welcome to C Real-Eval-Print-Loop (crepl).\n");
  printf("To exit, type \"exit\", \"quit\" or press Ctrl-D.\n\n");

  for (io_count = 1; ; ++io_count) {
    printf(" in[%d]: ", io_count);
    sprintf(output, "out[%d]: ", io_count);

    if ((input_size = getline((char **) &input, &buf_size, stdin)) <= 0) break;
    while (isspace(input[input_size - 1])) --input_size;
    Log("%s", input);

    switch (precheck()) {
      case TYPE_FUNC:
        if (compile(input, input_size)) {
          printf("%s", output); 
          printf("\33[0m" FG_GREEN "Loaded as function/variable %s.\n" "\033[0m", func_name);
        } else {
          printf("%s", output); 
          printf("\33[0m" FG_RED "Compilation error.\n" "\033[0m");
        }
        break;
      case TYPE_EVAL:
        if (calculate(input, input_size)) {
          printf("%s", output); 
          printf("\33[0m" FG_GREEN "Result: %d.\n" "\033[0m", calc_result);
        } else {
          printf("%s", output); 
          printf("\33[0m" FG_RED "Calculation error.\n" "\033[0m");
        }
        break;
      default:
        printf("%s", output); 
        printf("\33[0m" FG_BLUE "Good bye!\n" "\33[0m");
        return 0;
    }
    printf("\n");
  }
}

int precheck() {
  char first_word[8] = "";
  sscanf(input, " %7s", first_word);
  if (strcmp(first_word, "exit") == 0
      || strcmp(first_word, "quit") == 0) {
    return TYPE_EXIT;
  } else if (strcmp(first_word, "int") == 0) {
    return TYPE_FUNC;
  } else {
    return TYPE_EVAL;  
  }
}

bool compile(char *code, size_t size) {
  /* scan the name of the function */
  sscanf(code, " int %[^ (]", func_name);
  CLog(BG_PURPLE, "The name of func is %s.", func_name);

  /* create temporaty files */
  char name_src[32] = "SRC_XXXXXX";
  char name_dst[32] = "DST_XXXXXX";
  char file_src[128] = "";
  char file_dst[128] = "";
  int fd_src = mkstemp(name_src);
  int fd_dst = mkstemp(name_dst);
  Assert(fd_src >= 0, "Unable to create tempfile SRC.");
  Assert(fd_dst >= 0, "Unable to create tempfile DST.");
  unlink(name_src);
  unlink(name_dst);
  sprintf(file_src, "/proc/self/fd/%d", fd_src);
  sprintf(file_dst, "/proc/self/fd/%d", fd_dst);
  write(fd_src, code, size);
  CLog(BG_PURPLE, "Temporary files (%s, %s) created.", name_src, name_dst);

  char *CC_argv[] = {
    "gcc", 
    "-x", "c", // Language: C
    "-shared", // Compile shared-object file
    "-o", file_dst, 
    CC_ABI,    // ABI Type: 32 or 64 ?
    //"-Werror", // See all warnings as errors
    "-w",
    "-fPIC",   // Generate position independent code
    file_src,
    NULL
  };
  CLog(BG_PURPLE, "GCC's target ABI is %s.", CC_ABI);

  lib_handle = NULL; // reset this global var.
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
      if ((lib_handle = dlopen(file_dst, RTLD_LAZY | RTLD_GLOBAL)) == NULL) {
        CLog(BG_RED, "Failed to load dynamic library (dlopen).");
      } else {
        CLog(BG_GREEN, "Dynamic library %s loaded.", file_dst);
        success = true;
      }
    }
  }
  //close(fd_src);
  //close(fd_dst);
  return success;
}

bool calculate(char *code, size_t size) {
  bool success = false;
  char *real_code = func_wrapper(code, &size);
  if ((success = compile(real_code, size))) {
    Assert(lib_handle != NULL, "After compilation and loading, the handle is NULL.");
    CLog(BG_YELLOW, "Search func %s in %p.", func_name, lib_handle);
    void *func = dlsym(lib_handle, func_name);
    Assert(func != NULL, "The function pointer is NULL!");
    calc_result = ((int (*)()) func)();
    return true;
  }
  free(real_code);
  return success;
}

char *func_wrapper(char *code, size_t *size) {
  char *ret = malloc(sizeof(char) * (*size + 64));
  sprintf(ret, "int " FUNC_PREFIX "%d() {\n  return (%s);\n}", io_count, code);
  *size = strlen(ret);
  return ret;
}
