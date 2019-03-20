/**
 * FOR TEACHERS/TAs/CODE REVIERS:
 * HOW THIS PIECE OF CODE WORKS
 *
 * 1. Three States of Coroutines
 *    
 *    +- ST_S => 0 => Sleeping (unfinished)
 *    |- ST_R => 1 => Running (finished)
 *    `- ST_I => 2 => New CO, to be inited
 *    
 *    Please note that a running coroutine is
 *    actually a finished coroutine because it
 *    did not call co_yield() when the function
 *    returned. If the function will continue, 
 *    then it must call co_yield() to switch co-
 *    routines and thus it will be set sleeping.
 *
 * 2. The Workflow(Life) of a Coroutine
 *  1) In co_start(), create a new CO instance
 *     and add it to the end of linked list.
 *  2) Init CO by running it ONCE until the first 
 *     call of co_yield().
 *  3) In co_yield(), jump back to co_start() 
 *     and return the instance to the caller.
 *  4) When co_wait() is called, keep calling
 *     co_yield() until the target coroutine ends.
 *  5) When a coroutine ends, the program will
 *     continue in co_start() after where the
 *     first call was made. (MAGIC!) The use a 
 *     longjmp() to go back to co_wait() and then
 *     collect garbage of finished coroutines.
 *  6) When collecting garbage, any coroutine
 *     with running (ST_R => 1) state indicates
 *     that the function has finished, because
 *     an unfinished coroutine will call co_yield()
 *     to set its state as sleeping (ST_S => 0).
 *
 * 3. The STACK EXCHANGE (BAD PUN)
 *  1) Only switch to the original stack when the 
 *     program will leave functions in this file.
 *  2) When switching from one coroutine to another, 
 *     just exchange their Stack Pointers and call 
 *     longjmp().
 *
 * 4. Graph description:
 *    
 *      A => switch to an individual stack
 *      B => switch to the original stack
 *
 *     +----+-------'external caller'------+----+
 *     ^    |                              ^    |
 *     |    1            +<<<<<<+          |    5
 *     4    |           7|func()|          9    |
 *     |    v      A     +>>>>>>+    B     |    v
 *    co_start() --2--> co_yield() --8--> co_wait()
 *     ^                 |      ^               |
 *     |           B     |      |    A          |
 *     +-----------3-----+      +----6----------+
 */


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

void co_gc(struct co* thd) {
  if (!head) return;
  if (head == thd) {
    struct co* next = head->next;
    free(head);
    head = next;
  }
  if (head) {
    struct co* cp = head;
    while (cp->next) {
      if (cp->next == thd) {
        struct co* next = cp->next->next;
        free(cp->next);
        cp->next = next;
        break;
      }
      cp = cp->next;
    }
  }
  co_print();
}

struct co* co_create(const char *name, func_t func, void* arg) {
  struct co* ret = (struct co*) malloc(sizeof(struct co));
  ret->pid = ++co_cnt;
  ret->state = ST_I; // init state
  strncpy(ret->name, name, sizeof(ret->name));
  ret->func = func;
  ret->arg = arg;
  ret->next = NULL;
  ret->stack_ptr = (void *) ((((intptr_t) ret->stack + sizeof(ret->stack)) >> 4) << 4);
  if (head) {
    struct co* cp = head;
    while (cp->next) {
      Log("cp => %p", cp);
      cp = cp->next;
    }
    cp->next = ret;
  } else {
    head = ret;
  }
  co_print();
  return ret;
}

struct co* co_start(const char* name, func_t func, void* arg) {
  Log("%s START!", name);
  current = co_create(name, func, arg);
  if (!setjmp(start_buf)) {
    stackEX(current->stack_ptr, stack_backup);
    current->func(current->arg);
    longjmp(wait_buf, 1);
  } else {
    Log("init finished");
  }
  /* continue from co_yield */
  return current;
}

void co_yield() {
  if (!setjmp(current->buf)) {
    if (current->state == ST_I) {
      stackEX(stack_backup, current->stack_ptr);
      current->state = ST_S;
      longjmp(start_buf, 1);
      /* go back to co_start */
    } else {
      struct co* next = current->next ? current->next : head;
      current->state = ST_S;
      stackEX(next->stack_ptr, current->stack_ptr);
      current = next;
      longjmp(current->buf, 1);
    }
  } else {
    stackEX(current->stack_ptr, stack_backup);
    current->state = ST_R;
    Log("return to %s", current->name);
  }
}

void co_wait(struct co *thd) {
  Log("co_wait for %s!", thd->name);
  while (thd->state != ST_R) {
    if (!setjmp(wait_buf)) {
      current = thd;
      longjmp(current->buf, 1);
      /* will continue in co_start */
    }
    Log("One thread is finished!!");
    if (current == thd) break;
  }
  /* thd is finished */
  co_gc(thd);
  return;
}

void co_print() {
  Log("=====THREADS=====");
  for (struct co* cp = head; cp != NULL; cp = cp->next) {
    Log("%p => %d: [%s] %d", cp, cp->pid, cp->name, cp->state);
  }
  Log("=================");
}
