#ifndef __CO_H__
#define __CO_H__

#define SZ_STACK 4096
#define NR_CO 16

typedef void (*func_t)(void *arg);

void co_init();
struct co* co_start(const char *name, func_t func, void *arg);
void co_yield();
void co_wait(struct co *thd);

#endif
