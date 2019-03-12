#include <stdio.h>
#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <string.h>
#include "co.h"
#include "debug.h"

static int co_cnt = 0;

struct co {
  int pid;
  int state;
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

void co_gc() {
  if (!head) return;
  if (head->state == ST_R) {
    struct co* next = head->next;
    free(head);
    head = next;
  }
  struct co* cp = head;
  while (cp->next) {
    if (cp->next->state == ST_R) {
      struct co* next = cp->next->next;
      free(cp->next);
      cp->next = next;
    }
    cp = cp->next;
  }
}

struct co* co_start(const char *name, func_t func, void *arg) {
  Log("CO [%s] START!", name);
  struct co* cur = (struct co*) malloc(sizeof(struct co));
  cur->pid = ++co_cnt;
  cur->state = ST_S;
  strncpy(cur->name, name, strlen(cur->name));
  cur->next = NULL;
  if (head) {
    struct co* cp = head;
    while (cp->next != NULL) cp = cp->next;
    cp->next = cur;
  } else {
    head = cur;
  }
  return current = cur;
}

void co_yield() {
  int val = setjmp(current->buf);
  if (!val) {
    current->state = ST_S;
    current = current->next ? current->next : head;
    longjmp(current->buf, 1);
  } else {
    current->state = ST_R;
  }
}

void co_wait(struct co *thd) {
  while (thd->state != ST_R) co_yield();
  co_gc();
}

