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

enum task_states {
  ST_U, // Unused
  ST_E, // Embryo
  ST_T, // To sleep
  ST_S, // Sleeping 
  ST_W, // Waken up
  ST_R, // Running
  ST_Z, // Zombie
  ST_X  // Special
};

typedef struct task {
  uint32_t pid;
  const char* name;
  void (*entry)(void *);
  void *arg;

  enum task_states state;
  uint32_t owner;
  uint32_t count;
  _Context *context;

  char fenceA[32];
  char stack[4096];
  char fenceB[32];

  void *alarm;
  bool suicide;

  file_t *fildes[64];

  struct task *next;
} task_t;

struct task *get_current_task();
void set_current_task(struct task *);

void kmt_init();
int kmt_create(struct task *, const char *, void (*)(void *), void *);
void kmt_teardown(struct task *);
void kmt_inspect_fence(struct task *);
struct task *kmt_sched();
_Context *kmt_context_save(_Event, _Context *);
_Context *kmt_context_switch(_Event, _Context *);
_Context *kmt_timer(_Event, _Context *);
_Context *kmt_yield(_Event, _Context *);
_Context *kmt_error(_Event, _Context *);

#endif
