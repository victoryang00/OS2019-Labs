#ifndef __CO_H__
#define __CO_H__

#include <assert.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define DEBUG
#ifdef DEBUG
#include "debug.h"
#endif

#define SZ_STACK 1024
#define NR_CO 16

#define ST_S 0 // sleeping
#define ST_R 1 // running
#define ST_I 2 // init

typedef void (*func_t)(void *arg);
struct co {
  int pid;
  int state;
  char name[32];
  struct co* next;
  jmp_buf buf;
  char stack[SZ_STACK];
};

void co_init();
void co_gc();
struct co* co_start(const char *name, func_t func, void *arg);
void co_yield();
void co_wait(struct co *thd);
void co_print();

#ifdef __i386__
  #define SP "%%esp"
#else
  #define SP "%%rsp"
#endif

inline void stackON(struct co* cp, void* backup) {
  assert((intptr_t) cp->stack & 0x1 == 0);
  asm volatile("mov " SP ", %0" : "=g"(backup) :);
  asm volatile("mov %0, " SP : : "g"(cp->stack));
  Log("STACK ON!");
}

inline void stackOFF(void *backup) {
  asm volatile("mov %0, " SP : : "g"(backup));
  Log("STACK OFF!");
}

#endif
