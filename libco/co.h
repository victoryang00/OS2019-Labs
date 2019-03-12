#ifndef __CO_H__
#define __CO_H__

#define SZ_STACK 4096
#define NR_CO 16

#define ST_S 0 // sleeping
#define ST_R 1 // running
#define ST_I 2 // init

typedef void (*func_t)(void *arg);

void co_init();
void co_gc();
struct co* co_start(const char *name, func_t func, void *arg);
void co_yield();
void co_wait(struct co *thd);
void co_print();

#endif
