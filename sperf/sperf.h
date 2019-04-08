#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define DEBUG
#include "debug.h"

#define SZ_NAME 64

struct _perf_item {
  char name[SZ_NAME];
  double time;
  struct _perf_item *next;
}
typedef _perf_item perf_item;

extern double time_total;

void sperf(int, char *[]);
void child(int, int, char *[]);
void parent(int);
void addItem(char *, double);
void showItems();
