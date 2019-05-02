#ifndef __THREAD_H__
#define __THREAD_H__

#include <common.h>
#include <stdbool.h>

/**
 * Kernel Multi-thread module (KMT, Proc)
 */

enum task_stats = {
  ST_U, // Unused
  ST_E, // Embryo
  ST_S, // Sleeping 
  ST_W, // Waken up
  ST_R, // Running
  ST_Z  // Zombie/Orphan
};

struct task {
  int pid;
  size_t size;
  enum task_stats state;
  struct task *parent;
  struct context *context;
};

void proc_init();


#endif
