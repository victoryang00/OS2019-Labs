#ifndef __CO_H__
#define __CO_H__

#include <assert.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

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
  void* stack_ptr;
};

void co_init();
void co_gc();
struct co* co_create(const char *name);
struct co* co_start(const char *name, func_t func, void *arg);
void co_yield();
void co_wait(struct co *thd);
void co_print();

#if defined (__i386__)
  #define SP "%%esp"
#elif defined (__x86_64__)
  #define SP "%%rsp"
#endif

inline void stackON(struct co* cp, void* backup) {
  asm volatile("mov " SP ", %0" : "=g"(backup) :);
  asm volatile("mov %0, " SP : : "g"(cp->stack_ptr));
  Log("STACK ON!");
}

inline void stackOFF(void *backup) {
  asm volatile("mov %0, " SP : : "g"(backup));
  Log("STACK OFF!");
}

inline void pushParams(const char* a, func_t b, void* c) {
  /* push them in reverse order! */
  asm volatile("push %0" : : "g"((void*) c));
  asm volatile("push %0" : : "g"((void*) b));
  asm volatile("push %0" : : "g"((void*) a));
}

#endif
