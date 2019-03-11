#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <setjmp.h>
#include "co.h"

static int co_cnt = 0;

struct co {
  int pid;
  char name[32];
  jmp_buf buf;
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
  cur->next = NULL;
  if (head) {
    head = cur;
  } else {
    struct co* cp = head;
    while (cp->next != NULL) cp = cp->next;
    cp->next = cur;
  }
  return current = cur;
}

void co_yield() {
  struct co* cur = current;
  int val = setjmp(current->buf);
  if (!val) {
    /* ready to jump */
    struct co* next = current->next ? current->next : head;
    longjmp(next->buf, val);
  } else {
    /* back from jump */
    longjmp(cur->buf, val);
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

