#include <stdio.h>
#include <stdlib.h>
#include "co.h"

static int co_cnt = 0;

struct co {
  int pid;
  char name[32];
  uint8_t stack[SZ_STACK];
  struct co* next;
};
struct co* head = NULL;
struct co* current = NULL;

void co_init() {
  head = NULL;
  current = NULL;
}

struct co* co_start(const char *name, func_t func, void *arg) {
  struct co* cur = (struct co*) malloc(sizeof(struct co));
  cur->pid = co_cnt++;
  strncpy(cur->name, name, strlen(cur->name));
  memset(stack, 0, sizeof(stack));
  cur->next = NULL;
  } else {
    head = cur;
    current = cur;
  }
  return cur;
}

void co_yield() {
  int val = setjmp(current->stack);
  if (!val) {
    /* ready to jump */
    longjmp(current->next->stack);
  } else {
    /* back from jump */
    longjmp(current->next);
  }
}

void co_wait(struct co *thd) {
  co_yield(thd);
  struct co* cp = head;
  if (cp->pid == thd->pid) {
    head = thd->next;
  } else {
    while (cp->next && cp->next->pid != thd->pid) cp = cp->next;
    assert(cp->next); // co must be in the list
    cp->next = thd->next;
  }
  free(thd);
}

