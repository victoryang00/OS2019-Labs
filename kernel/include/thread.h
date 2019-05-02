#ifndef __THREAD_H__
#define __THREAD_H__

#include <common.h>
#include <stdbool.h>

/**
 * Kernel Multi-thread module (KMT, Proc)
 */

#define NR_TASKS 64
#define FILL_FENCE 0xcd
#define FILL_STACK 0xfd

enum task_stats {
  ST_U, // Unused
  ST_E, // Embryo
  ST_S, // Sleeping 
  ST_W, // Waken up
  ST_R, // Running
  ST_Z, // Zombie
  ST_X  // Special
};

struct task {
  int pid;
  const char* name;
  enum task_stats state;

  _Context context;
  char fenceA[32];
  char stack[4096];
  char fenceB[32];

  void *sleep_sem;
  struct spinlock *sleep_lock;

  struct task *next;
};

void kmt_init();
int kmt_create(struct task *, const char *, void (*)(void *), void *);
void kmt_teardown(struct task *);
void kmt_inspect_fence(struct task *);
//int kmt_wait();
void kmt_sched();
void kmt_yield();
void kmt_sleep(void *, struct spinlock *);
void kmt_wakeup(void *);

#endif
