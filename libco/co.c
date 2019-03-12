#include <stdio.h>
#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <string.h>
#include "co.h"
#include "debug.h"

static int co_cnt = 0;
static jmp_buf start_buf;
static jmp_buf wait_buf;

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
  cur->state = ST_I; // init state
  strncpy(cur->name, name, sizeof(cur->name));
  cur->next = NULL;
  if (head) {
    struct co* cp = head;
    while (cp->next != NULL) cp = cp->next;
    cp->next = cur;
  } else {
    head = cur;
  }
  current = cur;
  
  int val = setjmp(start_buf);
  if (val == 0) {
    func(arg);
    /* continue from co_wait */
    Log("FINISHEDDDD");
    longjmp(wait_buf, 1);
  }
  /* continue from co_yield */
  return cur;
}

void co_yield() {
  Log("co_yield called by CO [%s]!", current->name);
  int val = setjmp(current->buf);
  if (val == 0) {
    if (current->state == ST_I) {
      current->state = ST_S;
      longjmp(start_buf, 1);
      /* go back to co_start */
    } else {
      current->state = ST_S;
      current = current->next ? current->next : head;
      longjmp(current->buf, 1);
    }
  } else {
    current->state = ST_R;
    Log("Going back to CO [%s]", current->name);
  }
}

void co_wait(struct co *thd) {
  Log("co_wait for CO [%s]!", thd->name);
  while (thd->state != ST_R) {
    int val = setjmp(wait_buf);
    if (val == 0) {
      current = thd;
      longjmp(thd->buf, 1);
      /* if finished, will continue in co_start */
    }
  }
  return;
}

