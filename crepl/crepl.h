#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <dlfcn.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

//#define DEBUG
#include "debug.h"

#if defined (__i386__)
  #define CC_ABI "-m32"
#elif defined (__x86_64__)
  #define CC_ABI "-m64"
#endif

#define TYPE_INVAL 0
#define TYPE_FUNCT 1
#define TYPE_VALUE 2

#define FUNC_PREFIX "__EVAL_FUNC_"

void init();
int precheck();
bool compile(char *, size_t);
bool calculate(char *, size_t);
char *func_wrapper(char *, size_t *);
