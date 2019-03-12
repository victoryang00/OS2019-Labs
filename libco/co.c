#include <stdio.h>
#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <string.h>
#include "co.h"
#include "debug.h"

struct co {
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
  Log("CO [%s] START!", name);
  func(arg);
  return NULL;
}

void co_yield() {
  Log("Yield!");
  //struct co* cur = current;
  int val = setjmp(current->buf);
  if (!val) {
    /* ready to jump */
    current = current->next ? current->next : head;
    longjmp(current->buf, 1);
  } else {
    /* back from jump */
    //longjmp(cur->buf, 0);
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

