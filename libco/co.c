#include "co.h"

static void* stack_backup;
static int co_cnt = 0;
static jmp_buf start_buf;
static jmp_buf wait_buf;

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

struct co* co_create(const char *name, func_t func, void* arg) {
  struct co* ret = (struct co*) malloc(sizeof(struct co));
  ret->pid = ++co_cnt;
  ret->state = ST_I; // init state
  strncpy(ret->name, name, sizeof(ret->name));
  ret->func = func;
  ret->arg = arg;
  ret->next = NULL;
  memset(ret->buf, 0, sizeof(ret->buf));
  ret->stack_ptr = ret->stack + sizeof(ret->stack);
  if (head) {
    struct co* cp = head;
    while (cp->next != NULL) cp = cp->next;
    cp->next = ret;
  } else {
    head = ret;
  }
  co_print();
  return ret;
}

struct co* co_start(const char* name, func_t func, void* arg) {
  Log("CO [%s] START!", name);
  current = co_create(name, func, arg);
  if (!setjmp(start_buf)) {
    stack_backup = stackEX(current->stack_ptr);
    current->func(current->arg);
    /* continue from co_wait */
    Log("FINISHEDDDD");
    longjmp(wait_buf, 1);
  }
  /* continue from co_yield */
  return current;
}

void co_yield() {
  Log("co_yield called by CO [%s]!", current->name);
  if (!setjmp(current->buf)) {
    current->stack_ptr = stackEX(stack_backup);
    Log("[off] stack saved as %p", current->stack_ptr);
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
    Log("[on] set stack to %p", current->stack_ptr);
    stack_backup = stackEX(current->stack_ptr);
  }
}

void co_wait(struct co *thd) {
  Log("co_wait for CO [%s]!", thd->name);
  while (thd->state != ST_R) {
    if (!setjmp(wait_buf)) {
      longjmp(current->buf, 1);
      /* will continue in co_start */
    }
    /* one thread finished, but not thd */
    Log("One thread is finished!!");
  }
  /* thd is finished */
  return;
}

void co_print() {
  Log("=====THREADS=====");
  for (struct co* cp = head; cp != NULL; cp = cp->next) {
    Log("%p => %d: [%s] %d", cp, cp->pid, cp->name, cp->state);
  }
  Log("=================");
}
