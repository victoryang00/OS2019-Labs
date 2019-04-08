#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define DEBUG
#include "debug.h"

#define SZ_NAME 64
#define TM_FRAME 1

struct _perf_item {
  char call_name[SZ_NAME];
  double call_time;
  struct _perf_item *next;
};
typedef struct _perf_item perf_item;

extern int max_name_length;
extern double time_total;

void sperf(int, char *[]);
void child(int, int, char *[]);
void parent(int, int);
void addItem(char *, double);
void showItems();
